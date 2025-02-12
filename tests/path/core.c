#include "clar_libgit2.h"
#include "path.h"

void test_path_core__cleanup(void)
{
	cl_git_sandbox_cleanup();
}

static void test_make_relative(
	const char *expected_path,
	const char *path,
	const char *parent,
	int expected_status)
{
	git_buf buf = GIT_BUF_INIT;
	git_buf_puts(&buf, path);
	cl_assert_equal_i(expected_status, git_path_make_relative(&buf, parent));
	cl_assert_equal_s(expected_path, buf.ptr);
	git_buf_dispose(&buf);
}

void test_path_core__make_relative(void)
{
	test_make_relative("foo.c", "/path/to/foo.c", "/path/to", 0);
	test_make_relative("bar/foo.c", "/path/to/bar/foo.c", "/path/to", 0);
	test_make_relative("foo.c", "/path/to/foo.c", "/path/to/", 0);

	test_make_relative("", "/path/to", "/path/to", 0);
	test_make_relative("", "/path/to", "/path/to/", 0);

	test_make_relative("../", "/path/to", "/path/to/foo", 0);

	test_make_relative("../foo.c", "/path/to/foo.c", "/path/to/bar", 0);
	test_make_relative("../bar/foo.c", "/path/to/bar/foo.c", "/path/to/baz", 0);

	test_make_relative("../../foo.c", "/path/to/foo.c", "/path/to/foo/bar", 0);
	test_make_relative("../../foo/bar.c", "/path/to/foo/bar.c", "/path/to/bar/foo", 0);

	test_make_relative("../../foo.c", "/foo.c", "/bar/foo", 0);

	test_make_relative("foo.c", "/path/to/foo.c", "/path/to/", 0);
	test_make_relative("../foo.c", "/path/to/foo.c", "/path/to/bar/", 0);

	test_make_relative("foo.c", "d:/path/to/foo.c", "d:/path/to", 0);

	test_make_relative("../foo", "/foo", "/bar", 0);
	test_make_relative("path/to/foo.c", "/path/to/foo.c", "/", 0);
	test_make_relative("../foo", "path/to/foo", "path/to/bar", 0);

	test_make_relative("/path/to/foo.c", "/path/to/foo.c", "d:/path/to", GIT_ENOTFOUND);
	test_make_relative("d:/path/to/foo.c", "d:/path/to/foo.c", "/path/to", GIT_ENOTFOUND);

	test_make_relative("/path/to/foo.c", "/path/to/foo.c", "not-a-rooted-path", GIT_ENOTFOUND);
	test_make_relative("not-a-rooted-path", "not-a-rooted-path", "/path/to", GIT_ENOTFOUND);

	test_make_relative("/path", "/path", "pathtofoo", GIT_ENOTFOUND);
	test_make_relative("path", "path", "pathtofoo", GIT_ENOTFOUND);
}

void test_path_core__isvalid_standard(void)
{
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/bar", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/bar/file.txt", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/bar/.file", 0, 0));
}

void test_path_core__isvalid_empty_dir_component(void)
{
	cl_assert_equal_b(false, git_path_validate(NULL, "foo//bar", 0, 0));

	/* leading slash */
	cl_assert_equal_b(false, git_path_validate(NULL, "/", 0, 0));
	cl_assert_equal_b(false, git_path_validate(NULL, "/foo", 0, 0));
	cl_assert_equal_b(false, git_path_validate(NULL, "/foo/bar", 0, 0));

	/* trailing slash */
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/", 0, 0));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/bar/", 0, 0));
}

void test_path_core__isvalid_dot_and_dotdot(void)
{
	cl_assert_equal_b(true, git_path_validate(NULL, ".", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "./foo", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/.", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "./foo", 0, 0));

	cl_assert_equal_b(true, git_path_validate(NULL, "..", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "../foo", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/..", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "../foo", 0, 0));

	cl_assert_equal_b(false, git_path_validate(NULL, ".", 0, GIT_PATH_REJECT_TRAVERSAL));
	cl_assert_equal_b(false, git_path_validate(NULL, "./foo", 0, GIT_PATH_REJECT_TRAVERSAL));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/.", 0, GIT_PATH_REJECT_TRAVERSAL));
	cl_assert_equal_b(false, git_path_validate(NULL, "./foo", 0, GIT_PATH_REJECT_TRAVERSAL));

	cl_assert_equal_b(false, git_path_validate(NULL, "..", 0, GIT_PATH_REJECT_TRAVERSAL));
	cl_assert_equal_b(false, git_path_validate(NULL, "../foo", 0, GIT_PATH_REJECT_TRAVERSAL));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/..", 0, GIT_PATH_REJECT_TRAVERSAL));
	cl_assert_equal_b(false, git_path_validate(NULL, "../foo", 0, GIT_PATH_REJECT_TRAVERSAL));
}

void test_path_core__isvalid_dot_git(void)
{
	cl_assert_equal_b(true, git_path_validate(NULL, ".git", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, ".git/foo", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/.git", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/.git/bar", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/.GIT/bar", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/bar/.Git", 0, 0));

	cl_assert_equal_b(false, git_path_validate(NULL, ".git", 0, GIT_PATH_REJECT_DOT_GIT_LITERAL));
	cl_assert_equal_b(false, git_path_validate(NULL, ".git/foo", 0, GIT_PATH_REJECT_DOT_GIT_LITERAL));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/.git", 0, GIT_PATH_REJECT_DOT_GIT_LITERAL));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/.git/bar", 0, GIT_PATH_REJECT_DOT_GIT_LITERAL));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/.GIT/bar", 0, GIT_PATH_REJECT_DOT_GIT_LITERAL));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/bar/.Git", 0, GIT_PATH_REJECT_DOT_GIT_LITERAL));

	cl_assert_equal_b(true, git_path_validate(NULL, "!git", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/!git", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "!git/bar", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, ".tig", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/.tig", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, ".tig/bar", 0, 0));
}

void test_path_core__isvalid_backslash(void)
{
	cl_assert_equal_b(true, git_path_validate(NULL, "foo\\file.txt", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/bar\\file.txt", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/bar\\", 0, 0));

	cl_assert_equal_b(false, git_path_validate(NULL, "foo\\file.txt", 0, GIT_PATH_REJECT_BACKSLASH));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/bar\\file.txt", 0, GIT_PATH_REJECT_BACKSLASH));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/bar\\", 0, GIT_PATH_REJECT_BACKSLASH));
}

void test_path_core__isvalid_trailing_dot(void)
{
	cl_assert_equal_b(true, git_path_validate(NULL, "foo.", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo...", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/bar.", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo./bar", 0, 0));

	cl_assert_equal_b(false, git_path_validate(NULL, "foo.", 0, GIT_PATH_REJECT_TRAILING_DOT));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo...", 0, GIT_PATH_REJECT_TRAILING_DOT));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/bar.", 0, GIT_PATH_REJECT_TRAILING_DOT));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo./bar", 0, GIT_PATH_REJECT_TRAILING_DOT));
}

void test_path_core__isvalid_trailing_space(void)
{
	cl_assert_equal_b(true, git_path_validate(NULL, "foo ", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo   ", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/bar ", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, " ", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo /bar", 0, 0));

	cl_assert_equal_b(false, git_path_validate(NULL, "foo ", 0, GIT_PATH_REJECT_TRAILING_SPACE));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo   ", 0, GIT_PATH_REJECT_TRAILING_SPACE));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/bar ", 0, GIT_PATH_REJECT_TRAILING_SPACE));
	cl_assert_equal_b(false, git_path_validate(NULL, " ", 0, GIT_PATH_REJECT_TRAILING_SPACE));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo /bar", 0, GIT_PATH_REJECT_TRAILING_SPACE));
}

void test_path_core__isvalid_trailing_colon(void)
{
	cl_assert_equal_b(true, git_path_validate(NULL, "foo:", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo/bar:", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, ":", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "foo:/bar", 0, 0));

	cl_assert_equal_b(false, git_path_validate(NULL, "foo:", 0, GIT_PATH_REJECT_TRAILING_COLON));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo/bar:", 0, GIT_PATH_REJECT_TRAILING_COLON));
	cl_assert_equal_b(false, git_path_validate(NULL, ":", 0, GIT_PATH_REJECT_TRAILING_COLON));
	cl_assert_equal_b(false, git_path_validate(NULL, "foo:/bar", 0, GIT_PATH_REJECT_TRAILING_COLON));
}

void test_path_core__isvalid_dotgit_ntfs(void)
{
	cl_assert_equal_b(true, git_path_validate(NULL, ".git", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, ".git ", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, ".git.", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, ".git.. .", 0, 0));

	cl_assert_equal_b(true, git_path_validate(NULL, "git~1", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "git~1 ", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "git~1.", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "git~1.. .", 0, 0));

	cl_assert_equal_b(false, git_path_validate(NULL, ".git", 0, GIT_PATH_REJECT_DOT_GIT_NTFS));
	cl_assert_equal_b(false, git_path_validate(NULL, ".git ", 0, GIT_PATH_REJECT_DOT_GIT_NTFS));
	cl_assert_equal_b(false, git_path_validate(NULL, ".git.", 0, GIT_PATH_REJECT_DOT_GIT_NTFS));
	cl_assert_equal_b(false, git_path_validate(NULL, ".git.. .", 0, GIT_PATH_REJECT_DOT_GIT_NTFS));

	cl_assert_equal_b(false, git_path_validate(NULL, "git~1", 0, GIT_PATH_REJECT_DOT_GIT_NTFS));
	cl_assert_equal_b(false, git_path_validate(NULL, "git~1 ", 0, GIT_PATH_REJECT_DOT_GIT_NTFS));
	cl_assert_equal_b(false, git_path_validate(NULL, "git~1.", 0, GIT_PATH_REJECT_DOT_GIT_NTFS));
	cl_assert_equal_b(false, git_path_validate(NULL, "git~1.. .", 0, GIT_PATH_REJECT_DOT_GIT_NTFS));
}

void test_path_core__isvalid_dos_paths(void)
{
	cl_assert_equal_b(true, git_path_validate(NULL, "aux", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "aux.", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "aux:", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "aux.asdf", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "aux.asdf\\zippy", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "aux:asdf\\foobar", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "con", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "prn", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "nul", 0, 0));

	cl_assert_equal_b(false, git_path_validate(NULL, "aux", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "aux.", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "aux:", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "aux.asdf", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "aux.asdf\\zippy", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "aux:asdf\\foobar", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "con", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "prn", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "nul", 0, GIT_PATH_REJECT_DOS_PATHS));

	cl_assert_equal_b(true, git_path_validate(NULL, "aux1", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "aux1", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(true, git_path_validate(NULL, "auxn", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(true, git_path_validate(NULL, "aux\\foo", 0, GIT_PATH_REJECT_DOS_PATHS));
}

void test_path_core__isvalid_dos_paths_withnum(void)
{
	cl_assert_equal_b(true, git_path_validate(NULL, "com1", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "com1.", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "com1:", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "com1.asdf", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "com1.asdf\\zippy", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "com1:asdf\\foobar", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "com1\\foo", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "lpt1", 0, 0));

	cl_assert_equal_b(false, git_path_validate(NULL, "com1", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "com1.", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "com1:", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "com1.asdf", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "com1.asdf\\zippy", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "com1:asdf\\foobar", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "com1/foo", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(false, git_path_validate(NULL, "lpt1", 0, GIT_PATH_REJECT_DOS_PATHS));

	cl_assert_equal_b(true, git_path_validate(NULL, "com0", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "com0", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(true, git_path_validate(NULL, "com10", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "com10", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(true, git_path_validate(NULL, "comn", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(true, git_path_validate(NULL, "com1\\foo", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(true, git_path_validate(NULL, "lpt0", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(true, git_path_validate(NULL, "lpt10", 0, GIT_PATH_REJECT_DOS_PATHS));
	cl_assert_equal_b(true, git_path_validate(NULL, "lptn", 0, GIT_PATH_REJECT_DOS_PATHS));
}

void test_path_core__isvalid_nt_chars(void)
{
	cl_assert_equal_b(true, git_path_validate(NULL, "asdf\001foo", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "asdf\037bar", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "asdf<bar", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "asdf>foo", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "asdf:foo", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "asdf\"bar", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "asdf|foo", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "asdf?bar", 0, 0));
	cl_assert_equal_b(true, git_path_validate(NULL, "asdf*bar", 0, 0));

	cl_assert_equal_b(false, git_path_validate(NULL, "asdf\001foo", 0, GIT_PATH_REJECT_NT_CHARS));
	cl_assert_equal_b(false, git_path_validate(NULL, "asdf\037bar", 0, GIT_PATH_REJECT_NT_CHARS));
	cl_assert_equal_b(false, git_path_validate(NULL, "asdf<bar", 0, GIT_PATH_REJECT_NT_CHARS));
	cl_assert_equal_b(false, git_path_validate(NULL, "asdf>foo", 0, GIT_PATH_REJECT_NT_CHARS));
	cl_assert_equal_b(false, git_path_validate(NULL, "asdf:foo", 0, GIT_PATH_REJECT_NT_CHARS));
	cl_assert_equal_b(false, git_path_validate(NULL, "asdf\"bar", 0, GIT_PATH_REJECT_NT_CHARS));
	cl_assert_equal_b(false, git_path_validate(NULL, "asdf|foo", 0, GIT_PATH_REJECT_NT_CHARS));
	cl_assert_equal_b(false, git_path_validate(NULL, "asdf?bar", 0, GIT_PATH_REJECT_NT_CHARS));
	cl_assert_equal_b(false, git_path_validate(NULL, "asdf*bar", 0, GIT_PATH_REJECT_NT_CHARS));
}

void test_path_core__isvalid_dotgit_with_hfs_ignorables(void)
{
	cl_assert_equal_b(false, git_path_validate(NULL, ".git", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(false, git_path_validate(NULL, ".git\xe2\x80\x8c", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(false, git_path_validate(NULL, ".gi\xe2\x80\x8dT", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(false, git_path_validate(NULL, ".g\xe2\x80\x8eIt", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(false, git_path_validate(NULL, ".\xe2\x80\x8fgIt", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(false, git_path_validate(NULL, "\xe2\x80\xaa.gIt", 0, GIT_PATH_REJECT_DOT_GIT_HFS));

	cl_assert_equal_b(false, git_path_validate(NULL, "\xe2\x80\xab.\xe2\x80\xacG\xe2\x80\xadI\xe2\x80\xaet", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(false, git_path_validate(NULL, "\xe2\x81\xab.\xe2\x80\xaaG\xe2\x81\xabI\xe2\x80\xact", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(false, git_path_validate(NULL, "\xe2\x81\xad.\xe2\x80\xaeG\xef\xbb\xbfIT", 0, GIT_PATH_REJECT_DOT_GIT_HFS));

	cl_assert_equal_b(true, git_path_validate(NULL, ".", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, ".g", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, ".gi", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, " .git", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, "..git\xe2\x80\x8c", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, ".gi\xe2\x80\x8dT.", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, ".g\xe2\x80It", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, ".\xe2gIt", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, "\xe2\x80\xaa.gi", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, ".gi\x80\x8dT", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, ".gi\x8dT", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, ".g\xe2i\x80T\x8e", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, ".git\xe2\x80\xbf", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
	cl_assert_equal_b(true, git_path_validate(NULL, ".git\xe2\xab\x81", 0, GIT_PATH_REJECT_DOT_GIT_HFS));
}

void test_path_core__validate_workdir(void)
{
	cl_must_pass(git_path_validate_workdir(NULL, "/foo/bar"));
	cl_must_pass(git_path_validate_workdir(NULL, "C:\\Foo\\Bar"));
	cl_must_pass(git_path_validate_workdir(NULL, "\\\\?\\C:\\Foo\\Bar"));
	cl_must_pass(git_path_validate_workdir(NULL, "\\\\?\\C:\\Foo\\Bar"));
	cl_must_pass(git_path_validate_workdir(NULL, "\\\\?\\UNC\\server\\C$\\folder"));

#ifdef GIT_WIN32
	/*
	 * In the absense of a repo configuration, 259 character paths
	 * succeed. >= 260 character paths fail.
	 */
	cl_must_pass(git_path_validate_workdir(NULL, "C:\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\ok.txt"));
	cl_must_pass(git_path_validate_workdir(NULL, "C:\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\260.txt"));
	cl_must_fail(git_path_validate_workdir(NULL, "C:\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\longer_than_260.txt"));

	/* count characters, not bytes */
	cl_must_pass(git_path_validate_workdir(NULL, "C:\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\\260.txt"));
	cl_must_fail(git_path_validate_workdir(NULL, "C:\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\\long.txt"));
#else
	cl_must_pass(git_path_validate_workdir(NULL, "/c/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/ok.txt"));
	cl_must_pass(git_path_validate_workdir(NULL, "/c/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/260.txt"));
	cl_must_pass(git_path_validate_workdir(NULL, "/c/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/longer_than_260.txt"));
	cl_must_pass(git_path_validate_workdir(NULL, "C:\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\\260.txt"));
	cl_must_pass(git_path_validate_workdir(NULL, "C:\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\aaaaaaaaa\\\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\xc2\xa2\\long.txt"));
#endif
}

void test_path_core__validate_workdir_with_core_longpath(void)
{
#ifdef GIT_WIN32
	git_repository *repo;
	git_config *config;

	repo = cl_git_sandbox_init("empty_bare.git");

	cl_git_pass(git_repository_open(&repo, "empty_bare.git"));
	cl_git_pass(git_repository_config(&config, repo));

	/* fail by default */
	cl_must_fail(git_path_validate_workdir(repo, "/c/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/longer_than_260.txt"));

	/* set core.longpaths explicitly on */
	cl_git_pass(git_config_set_bool(config, "core.longpaths", 1));
	cl_must_pass(git_path_validate_workdir(repo, "/c/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/longer_than_260.txt"));

	/* set core.longpaths explicitly off */
	cl_git_pass(git_config_set_bool(config, "core.longpaths", 0));
	cl_must_fail(git_path_validate_workdir(repo, "/c/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/aaaaaaaaa/longer_than_260.txt"));
#endif
}

static void test_join_unrooted(
	const char *expected_result,
	ssize_t expected_rootlen,
	const char *path,
	const char *base)
{
	git_buf result = GIT_BUF_INIT;
	ssize_t root_at;

	cl_git_pass(git_path_join_unrooted(&result, path, base, &root_at));
	cl_assert_equal_s(expected_result, result.ptr);
	cl_assert_equal_i(expected_rootlen, root_at);

	git_buf_dispose(&result);
}

void test_path_core__join_unrooted(void)
{
	git_buf out = GIT_BUF_INIT;

	test_join_unrooted("foo", 0, "foo", NULL);
	test_join_unrooted("foo/bar", 0, "foo/bar", NULL);

	/* Relative paths have base prepended */
	test_join_unrooted("/foo/bar", 4, "bar", "/foo");
	test_join_unrooted("/foo/bar/foobar", 4, "bar/foobar", "/foo");
	test_join_unrooted("c:/foo/bar/foobar", 6, "bar/foobar", "c:/foo");
	test_join_unrooted("c:/foo/bar/foobar", 10, "foobar", "c:/foo/bar");

	/* Absolute paths are not prepended with base */
	test_join_unrooted("/foo", 0, "/foo", "/asdf");
	test_join_unrooted("/foo/bar", 0, "/foo/bar", "/asdf");

	/* Drive letter is given as root length on Windows */
	test_join_unrooted("c:/foo", 2, "c:/foo", "c:/asdf");
	test_join_unrooted("c:/foo/bar", 2, "c:/foo/bar", "c:/asdf");

#ifdef GIT_WIN32
	/* Paths starting with '\\' are absolute */
	test_join_unrooted("\\bar", 0, "\\bar", "c:/foo/");
	test_join_unrooted("\\\\network\\bar", 9, "\\\\network\\bar", "c:/foo/");
#else
	/* Paths starting with '\\' are not absolute on non-Windows systems */
	test_join_unrooted("/foo/\\bar", 4, "\\bar", "/foo");
	test_join_unrooted("c:/foo/\\bar", 7, "\\bar", "c:/foo/");
#endif

	/* Base is returned when it's provided and is the prefix */
	test_join_unrooted("c:/foo/bar/foobar", 6, "c:/foo/bar/foobar", "c:/foo");
	test_join_unrooted("c:/foo/bar/foobar", 10, "c:/foo/bar/foobar", "c:/foo/bar");

	/* Trailing slash in the base is ignored */
	test_join_unrooted("c:/foo/bar/foobar", 6, "c:/foo/bar/foobar", "c:/foo/");

	git_buf_dispose(&out);
}

void test_path_core__join_unrooted_respects_funny_windows_roots(void)
{
	test_join_unrooted("💩:/foo/bar/foobar", 9, "bar/foobar", "💩:/foo");
	test_join_unrooted("💩:/foo/bar/foobar", 13, "foobar", "💩:/foo/bar");
	test_join_unrooted("💩:/foo", 5, "💩:/foo", "💩:/asdf");
	test_join_unrooted("💩:/foo/bar", 5, "💩:/foo/bar", "💩:/asdf");
	test_join_unrooted("💩:/foo/bar/foobar", 9, "💩:/foo/bar/foobar", "💩:/foo");
	test_join_unrooted("💩:/foo/bar/foobar", 13, "💩:/foo/bar/foobar", "💩:/foo/bar");
	test_join_unrooted("💩:/foo/bar/foobar", 9, "💩:/foo/bar/foobar", "💩:/foo/");
}
