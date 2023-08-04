// Copyright Maarten L. Hekkelman, Radboud University 2008-2013.
//        Copyright Maarten L. Hekkelman, 2014-2022
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/included/unit_test.hpp>

#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <gxrio.hpp>

namespace tt = boost::test_tools;
namespace fs = std::filesystem;

fs::path gTestDir = fs::current_path(); // filled in first test

#if HAVE_LibLZMA
unsigned char kXZData[] = {
	0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00, 0x00, 0x04, 0xe6, 0xd6, 0xb4, 0x46, 0x02, 0x00, 0x21, 0x01, 
	0x16, 0x00, 0x00, 0x00, 0x74, 0x2f, 0xe5, 0xa3, 0x01, 0x00, 0x0d, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 
	0x2c, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0x0a, 0x00, 0x00, 0x00, 0xc3, 0xad, 0x94, 0xb3, 
	0x17, 0xf6, 0x0c, 0xca, 0x00, 0x01, 0x26, 0x0e, 0x08, 0x1b, 0xe0, 0x04, 0x1f, 0xb6, 0xf3, 0x7d, 
	0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0x59, 0x5a
};
#endif

unsigned char kGZippedData[] = {
	0x1f, 0x8b, 0x08, 0x08, 0x61, 0xb2, 0xf0, 0x62, 0x00, 0x03, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74,
	0x78, 0x74, 0x00, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0xd7, 0x51, 0x28, 0xcf, 0x2f, 0xca, 0x49, 0x51,
	0xe4, 0x02, 0x00, 0x18, 0xa7, 0x55, 0x7b, 0x0e, 0x00, 0x00, 0x00
};

// --------------------------------------------------------------------

bool init_unit_test()
{
	// not a test, just initialize test dir
	if (boost::unit_test::framework::master_test_suite().argc == 2)
		gTestDir = boost::unit_test::framework::master_test_suite().argv[1];
	else if (fs::current_path().filename().string() == "Debug" or fs::current_path().filename().string() == "Release")
		gTestDir = fs::current_path().parent_path().parent_path() / "test";

	return true;
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_1)
{
	for (fs::path f : {
		gTestDir / "hello.txt.gz",
#if HAVE_LibLZMA
		gTestDir / "hello.txt.xz",
#endif
		gTestDir / "hello.txt" })
	{
		std::filebuf fb;
		fb.open(f, std::ios::in | std::ios::binary);
		BOOST_CHECK(fb.is_open());

		gxrio::istream is(&fb);

		std::string line;

		getline(is, line);

		BOOST_CHECK_EQUAL(line, "Hello, world!");
	}
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_2)
{
	for (fs::path f : {
		gTestDir / "hello.txt.gz",
#if HAVE_LibLZMA
		gTestDir / "hello.txt.xz",
#endif
		gTestDir / "hello.txt" })
	{
		gxrio::ifstream is(f);
		BOOST_CHECK(is.is_open());

		std::string line;

		getline(is, line);

		BOOST_CHECK_EQUAL(line, "Hello, world!");
	}
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_3)
{
	for (fs::path f : {
		gTestDir / "hello.txt.gz",
#if HAVE_LibLZMA
		gTestDir / "hello.txt.xz",
#endif
		gTestDir / "hello.txt" })
	{
		gxrio::ifstream is1(f);
		BOOST_CHECK(is1.is_open());

		gxrio::ifstream is2(std::move(is1));
		BOOST_CHECK(is2.is_open());

		std::string line;

		getline(is2, line);

		BOOST_CHECK_EQUAL(line, "Hello, world!");
	}
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_4)
{
	for (fs::path f : {
		gTestDir / "hello.txt.gz",
#if HAVE_LibLZMA
		gTestDir / "hello.txt.xz",
#endif
		gTestDir / "hello.txt" })
	{
		gxrio::ifstream is1(f);
		BOOST_CHECK(is1.is_open());

		gxrio::ifstream is2;
		
		is2 = std::move(is1);
		BOOST_CHECK(is2.is_open());

		std::string line;

		getline(is2, line);

		BOOST_CHECK_EQUAL(line, "Hello, world!");
	}
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_5)
{
	std::filesystem::create_directories(std::filesystem::temp_directory_path() / "gxrio-unit-test");

	for (fs::path f : {
		std::filesystem::temp_directory_path() / "gxrio-unit-test" / "hello.txt.gz",
#if HAVE_LibLZMA
		std::filesystem::temp_directory_path() / "gxrio-unit-test" / "hello.txt.xz",
#endif
		std::filesystem::temp_directory_path() / "gxrio-unit-test" / "hello.txt" })
	{
		gxrio::ofstream out(f);
		BOOST_CHECK(out.is_open());

		out << "Hello, world!" << std::endl;
		out.close();

		gxrio::ifstream in(f);
		BOOST_CHECK(in.is_open());

		std::string line;

		getline(in, line);

		BOOST_CHECK_EQUAL(line, "Hello, world!");
	}
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_6)
{
	std::filesystem::create_directories(std::filesystem::temp_directory_path() / "gxrio-unit-test");

	for (fs::path f : {
		std::filesystem::temp_directory_path() / "gxrio-unit-test" / "hello.txt.gz",
#if HAVE_LibLZMA
		std::filesystem::temp_directory_path() / "gxrio-unit-test" / "hello.txt.xz",
#endif
		std::filesystem::temp_directory_path() / "gxrio-unit-test" / "hello.txt" })
	{
		gxrio::ofstream out_1(f);
		BOOST_CHECK(out_1.is_open());

		gxrio::ofstream out_2(std::move(out_1));
		BOOST_CHECK(out_2.is_open());

		out_2 << "Hello, world!" << std::endl;
		out_2.close();

		gxrio::ifstream in(f);

		std::string line;

		getline(in, line);

		BOOST_CHECK_EQUAL(line, "Hello, world!");
	}
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_7)
{
	std::filesystem::create_directories(std::filesystem::temp_directory_path() / "gxrio-unit-test");

	for (fs::path f : {
		std::filesystem::temp_directory_path() / "gxrio-unit-test" / "hello.txt.gz",
#if HAVE_LibLZMA
		std::filesystem::temp_directory_path() / "gxrio-unit-test" / "hello.txt.xz",
#endif
		std::filesystem::temp_directory_path() / "gxrio-unit-test" / "hello.txt" })
	{
		gxrio::ofstream out_1(f);
		BOOST_CHECK(out_1.is_open());

		gxrio::ofstream out_2;
		out_2 = std::move(out_1);
		BOOST_CHECK(out_2.is_open());

		out_2 << "Hello, world!" << std::endl;
		out_2.close();

		gxrio::ifstream in(f);

		std::string line;

		getline(in, line);

		BOOST_CHECK_EQUAL(line, "Hello, world!");
	}
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_8)
{
	for (const auto &[text, length] : std::vector<std::tuple<const char *, size_t>>{
		{ "Hello, world!", 13 },
		{ (const char*)kGZippedData, sizeof(kGZippedData) },
#if HAVE_LibLZMA
		{ (const char*)kXZData, sizeof(kXZData) },
#endif
		{ "\xfd\x37Hello, world!", 15 },
		{ "\x1fHello, world!", 14 }
	 })
	{
		struct membuf : public std::streambuf
		{
			membuf(char * text, size_t length)
			{
				this->setg(text, text, text + length);
			}
		} buffer(const_cast<char *>(text), length);

		gxrio::istream in(&buffer);

		std::string line;
		std::getline(in, line);

		BOOST_CHECK(line.length() >= 13);
		if (line.length() < 13)
			continue;

		BOOST_CHECK(line.compare(line.length() - 13, 13, "Hello, world!") == 0);
	}
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_9)
{
	gxrio::ifstream file(gTestDir / "aap.gz");

	BOOST_ASSERT(file.is_open());

	std::string line;

	BOOST_CHECK(getline(file, line));
	BOOST_TEST(line == "aap");

	BOOST_CHECK(getline(file, line));
	BOOST_TEST(line == "noot");

	BOOST_CHECK(getline(file, line));
	BOOST_TEST(line == "mies");

	BOOST_CHECK(not getline(file, line));
	BOOST_CHECK(file.eof());
}