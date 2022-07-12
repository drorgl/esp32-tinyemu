#include <unity.h>
#include <runner.h>

#include <virtual_directory.h>

void setUp()
{
}
void tearDown()
{
}


void chdir_when_changing_to_subdirectory_the_full_path_should_be_in_the_subdirectory()
{
    const char * home_cwd = "/home";
    vd_chdir(home_cwd);

    const char * subdirectory = "sub";

    vd_chdir(subdirectory);

    char cwd[255] = {0};
    vd_cwd(cwd, sizeof(cwd));

    TEST_ASSERT_EQUAL_STRING("/home/sub", cwd);
}

void chdir_when_changing_to_root_directory_the_root_directory_should_be_the_cwd()
{
    const char * home_cwd = "/home";
    vd_chdir(home_cwd);

    const char * root_cwd = "/2nd_home";

    vd_chdir(root_cwd);

    char cwd[255] = {0};
    vd_cwd(cwd, sizeof(cwd));

    TEST_ASSERT_EQUAL_STRING("/2nd_home", cwd);
}

void chdir_when_changing_to_parent_the_path_should_not_contain_the_dottot()
{
    const char * home_cwd = "/home/sub";
    vd_chdir(home_cwd);

    const char * parent_cwd = "../";

    vd_chdir(parent_cwd);

    char cwd[255] = {0};
    vd_cwd(cwd, sizeof(cwd));

    TEST_ASSERT_EQUAL_STRING("/home/", cwd);
}

void chdir_when_changing_to_parent_but_the_parent_is_root_the_path_should_not_contain_the_dottot()
{
    const char * home_cwd = "/home/sub";
    vd_chdir(home_cwd);

    const char * parent_cwd = "../../";

    vd_chdir(parent_cwd);

    char cwd[255] = {0};
    vd_cwd(cwd, sizeof(cwd));

    TEST_ASSERT_EQUAL_STRING("/", cwd);
}

void chdir_when_changing_to_parent_but_the_parent_is_already_root_the_path_should_not_contain_the_dottot()
{
    const char * home_cwd = "/home/sub";
    vd_chdir(home_cwd);

    const char * parent_cwd = "../../../";

    vd_chdir(parent_cwd);

    char cwd[255] = {0};
    vd_cwd(cwd, sizeof(cwd));

    TEST_ASSERT_EQUAL_STRING("/", cwd);
}

void chdir_when_changing_to_parent_and_subdirectory_the_path_should_be_the_parent_and_subdirectory()
{
    const char * home_cwd = "/home/sub";
    vd_chdir(home_cwd);

    const char * parent_cwd = "../sub2/";

    vd_chdir(parent_cwd);

    char cwd[255] = {0};
    vd_cwd(cwd, sizeof(cwd));

    TEST_ASSERT_EQUAL_STRING("/home/sub2/", cwd);
}

void chdir_when_changing_to_current_and_subdirectory_the_path_should_be_the_current_and_subdirectory()
{
    const char * home_cwd = "/home/sub";
    vd_chdir(home_cwd);

    const char * parent_cwd = "./sub2/";

    vd_chdir(parent_cwd);

    char cwd[255] = {0};
    vd_cwd(cwd, sizeof(cwd));

    TEST_ASSERT_EQUAL_STRING("/home/sub/sub2/", cwd);
}

MAIN()
{
    UNITY_BEGIN(); // IMPORTANT LINE!
    RUN_TEST(chdir_when_changing_to_subdirectory_the_full_path_should_be_in_the_subdirectory);
    RUN_TEST(chdir_when_changing_to_root_directory_the_root_directory_should_be_the_cwd);
    RUN_TEST(chdir_when_changing_to_parent_the_path_should_not_contain_the_dottot);
    RUN_TEST(chdir_when_changing_to_parent_but_the_parent_is_root_the_path_should_not_contain_the_dottot);
    RUN_TEST(chdir_when_changing_to_parent_but_the_parent_is_already_root_the_path_should_not_contain_the_dottot);
    RUN_TEST(chdir_when_changing_to_parent_and_subdirectory_the_path_should_be_the_parent_and_subdirectory);
    RUN_TEST(chdir_when_changing_to_current_and_subdirectory_the_path_should_be_the_current_and_subdirectory);
    UNITY_END(); // stop unit testing
}