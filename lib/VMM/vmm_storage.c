#include <vmm_storage.h>

#include <stddef.h>
#include <assert.h>
#include <log.h>

#include <time.h>
#include <bitmasker.h>
#include <string.h>
#include <math.h>

#include <sys/time.h>

struct VMM_storage_t *vmm_storage_create(const char *pagefile, size_t page_size, size_t maximum_size)
{
    log_info("creating VMM storage %s size: %zu, page: %zu", pagefile, maximum_size, page_size);

    struct VMM_storage_t *vmm_storage = malloc(sizeof(struct VMM_storage_t));

    if (!vmm_storage)
    {
        log_error("malloc() failed");
        return NULL;
    }

    vmm_storage->maximum_size = maximum_size;
    vmm_storage->page_size = page_size;

    vmm_storage->section_shift = log2(vmm_storage->page_size / 8);
    log_debug("section shift %d", vmm_storage->section_shift);
    vmm_storage->page_dirty = (uint8_t *)malloc(sizeof(uint8_t) * maximum_size / page_size);
    log_debug("allocating %zu bytes for dirty tracking", sizeof(uint8_t) * maximum_size / page_size);
    memset(vmm_storage->page_dirty, 0, sizeof(uint8_t) * maximum_size / page_size);

    log_debug("allocating %zu bytes for write dirty tracking", sizeof(uint8_t) * maximum_size / page_size);
    vmm_storage->dirty_write_sections = (uint8_t *)malloc(sizeof(uint8_t) * maximum_size / page_size);
    memset(vmm_storage->dirty_write_sections, 0, sizeof(uint8_t) * maximum_size / page_size);

    strcpy((char *)&vmm_storage->filename, pagefile);

    FILE *create_f = fopen(pagefile, "wb");
    if (!create_f)
    {
        log_error("fopen() failed");
    }
    if (setvbuf(create_f, NULL, _IOFBF, 1024 * 4) != 0)
    {
        perror("setvbuf");
    }
    if (fseek(create_f, maximum_size, SEEK_SET) != 0)
    {
        log_error("fseek() failed");
    }
    if (fputc('\0', create_f) != '\0')
    {
        log_error("fputc() failed");
    }
    fclose(create_f);

    log_trace("opening for rw");

    vmm_storage->backing_store = fopen(pagefile, "r+b");
    if (!vmm_storage->backing_store)
    {
        log_error("error opening %s file", vmm_storage->filename);
        vmm_storage_destroy(vmm_storage);
        return NULL;
    }
    if (setvbuf(vmm_storage->backing_store, NULL, _IOFBF, 1024 * 8) != 0)
    {
        perror("setvbuf");
    }

    log_debug("opened");

    vmm_storage->statistics.store_writes = 0;
    vmm_storage->statistics.store_write_bytes = 0;
    vmm_storage->statistics.store_write_disk_bytes = 0;
    vmm_storage->statistics.store_write_time = 0;

    vmm_storage->statistics.store_reads = 0;
    vmm_storage->statistics.store_read_bytes = 0;
    vmm_storage->statistics.store_read_disk_bytes = 0;
    vmm_storage->statistics.store_read_time = 0;

    return vmm_storage;
}

void vmm_storage_destroy(struct VMM_storage_t *vmm_storage)
{
    log_debug("destroying VMM storage");
    if (vmm_storage->backing_store)
    {
        fclose(vmm_storage->backing_store);
        vmm_storage->backing_store = NULL;
    }
    free(vmm_storage->page_dirty);
    free(vmm_storage->dirty_write_sections);
    free(vmm_storage);
}

struct enumerate_info_t
{
    struct VMM_storage_t *vmm_storage;
    size_t page_number;
    uint8_t *buf;
};

static void write_to_disk(int start, int end, void *user_data)
{
    struct enumerate_info_t *write_info = user_data;
    log_debug("writing range %d-%d page %zu", start, end, write_info->page_number);

    size_t start_offset = start << write_info->vmm_storage->section_shift;
    size_t end_offset = (end + 1) << write_info->vmm_storage->section_shift;

    if (fseek(write_info->vmm_storage->backing_store, write_info->page_number * write_info->vmm_storage->page_size + start_offset, SEEK_SET) != 0)
    {
        log_warn("Error seeking in backing store");
    }

    if (fwrite(write_info->buf + start_offset, sizeof(uint8_t), end_offset - start_offset, write_info->vmm_storage->backing_store) != (end_offset - start_offset))
    {
        log_error("Error writing to backing store\n");
    }

#ifdef VMM_DEBUG
    write_info->vmm_storage->statistics.store_write_disk_bytes += end_offset - start_offset;
#endif
}

void vmm_storage_write(struct VMM_storage_t *vmm_storage, size_t page_number, const void *buf)
{
#ifdef VMM_DEBUG
    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
#endif
    log_debug("writing page %zu to VMM storage %s", page_number, vmm_storage->filename);
    assert(vmm_storage->dirty_write_sections[page_number] != 0 && "vmm_storage_write called on a page that is not dirty write");
    struct enumerate_info_t write_info = {
        .vmm_storage = vmm_storage,
        .page_number = page_number,
        .buf = (uint8_t *)buf};

    enumerate_bit_ranges(vmm_storage->dirty_write_sections[page_number], write_to_disk, &write_info);

    vmm_storage->dirty_write_sections[page_number] = 0;

#ifdef VMM_DEBUG
    struct timeval tv_end;
    gettimeofday(&tv_end, NULL);

    float t_s = tv_end.tv_sec - tv_start.tv_sec + 1e-6f * (tv_end.tv_usec - tv_start.tv_usec);
    vmm_storage->statistics.store_write_time += (t_s * 1e3);

    vmm_storage->statistics.store_writes++;
    vmm_storage->statistics.store_write_bytes += vmm_storage->page_size;
#endif
}

static void read_from_disk(int start, int end, void *user_data)
{
    struct enumerate_info_t *read_info = user_data;

    size_t start_offset = start << read_info->vmm_storage->section_shift;
    size_t end_offset = (end + 1) << read_info->vmm_storage->section_shift;

    if (fseek(read_info->vmm_storage->backing_store, read_info->page_number * read_info->vmm_storage->page_size + start_offset, SEEK_SET) != 0)
    {
        log_error("Error seeking in backing store, page %d\n", read_info->page_number);
        *(uint8_t *)(0) = 1;
    }

    if (fread(read_info->buf + start_offset, sizeof(uint8_t), end_offset - start_offset, read_info->vmm_storage->backing_store) == 0)
    {
        log_error("Error reading from backing store\n");
    }

#ifdef VMM_DEBUG
    read_info->vmm_storage->statistics.store_read_disk_bytes += end_offset - start_offset;
#endif
}

void vmm_storage_read(struct VMM_storage_t *vmm_storage, size_t page_number, uint8_t *buf)
{
#ifdef VMM_DEBUG
    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
#endif
    log_debug("reading page %zu from VMM storage %s", page_number, vmm_storage->filename);
    assert(vmm_storage->page_dirty[page_number] != 0 && "vmm_storage_read called on a page that has not been written to yet");

    size_t buf_len = vmm_storage->page_size;
    memset(buf, 0, buf_len); // this needs to be here because the kernel read uninitialized memory!!!
    if (vmm_storage->page_dirty[page_number] == 0)
    {
        log_debug("skipping read of page %d, empty (%zu)", page_number, buf_len);
        return;
    }

    struct enumerate_info_t read_info = {
        .vmm_storage = vmm_storage,
        .page_number = page_number,
        .buf = buf};
    enumerate_bit_ranges(vmm_storage->page_dirty[page_number], read_from_disk, &read_info);

#ifdef VMM_DEBUG
    struct timeval tv_end;
    gettimeofday(&tv_end, NULL);

    float t_s = tv_end.tv_sec - tv_start.tv_sec + 1e-6f * (tv_end.tv_usec - tv_start.tv_usec);
    vmm_storage->statistics.store_read_time += (t_s * 1e3);

    vmm_storage->statistics.store_reads++;
    vmm_storage->statistics.store_read_bytes += buf_len;
#endif
}

void vmm_storage_set_page_dirty(struct VMM_storage_t *vmm_storage, size_t page_number, size_t offset, size_t length){
    const uint8_t section_mask = get_section_mask(vmm_storage->section_shift, offset, length);
    vmm_storage->page_dirty[page_number] |=section_mask;
    vmm_storage->dirty_write_sections[page_number] |= section_mask;
}

uint8_t vmm_storage_get_page_dirty(struct VMM_storage_t *vmm_storage, size_t page_number){
    return vmm_storage->page_dirty[page_number];
}
uint8_t vmm_storage_get_page_write_dirty(struct VMM_storage_t *vmm_storage, size_t page_number){
    return vmm_storage->dirty_write_sections[page_number];
}

void vmm_storage_clear_page_dirty(struct VMM_storage_t *vmm_storage, size_t page_number){
    log_debug("clearing dirty page %d", page_number);
    assert(vmm_storage->dirty_write_sections[page_number] != 0 && "vmm_storage_clear_page_dirty called on a page that has not been flushed");
    vmm_storage->page_dirty[page_number] = 0; //reset dirty bit if the page is all zeroes
    vmm_storage->dirty_write_sections[page_number] = 0; //reset dirty write sections
}


void vmm_storage_flush(struct VMM_storage_t *vmm_storage){
    log_debug("flushing VMM storage %s", vmm_storage->filename);
    fflush(vmm_storage->backing_store);
}