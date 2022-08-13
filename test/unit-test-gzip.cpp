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

unsigned char kGZippedData[] = {
	0x1f, 0x8b, 0x08, 0x08, 0x61, 0xb2, 0xf0, 0x62, 0x00, 0x03, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74,
	0x78, 0x74, 0x00, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0xd7, 0x51, 0x28, 0xcf, 0x2f, 0xca, 0x49, 0x51,
	0xe4, 0x02, 0x00, 0x18, 0xa7, 0x55, 0x7b, 0x0e, 0x00, 0x00, 0x00
};

static_assert(sizeof(kGZippedData) == 43, "That buffer should be 43 bytes in length");

// --------------------------------------------------------------------

bool init_unit_test()
{
	// not a test, just initialize test dir
	if (boost::unit_test::framework::master_test_suite().argc == 2)
		gTestDir = boost::unit_test::framework::master_test_suite().argv[1];

	return true;
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_c)
{
	struct membuf : public std::streambuf
	{
		membuf(char *text, size_t length)
		{
			this->setg(text, text, text + length);
		}
	} buffer(reinterpret_cast<char *>(kGZippedData), sizeof(kGZippedData));

	gxrio::gzip::istream in(&buffer);

	std::string line;
	std::getline(in, line);

	BOOST_CHECK_EQUAL(line, "Hello, world!");
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_1)
{
	fs::path f = gTestDir / "hello.txt.gz";

	gxrio::gzip::ifstream in(f, std::ios::in | std::ios::binary);

	std::string line;
	std::getline(in, line);

	BOOST_CHECK_EQUAL(line, "Hello, world!");

}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_100)
{
	fs::path f = gTestDir / "hello-100.txt.gz";

	gxrio::gzip::ifstream in(f, std::ios::in | std::ios::binary);

	std::string line;

	int n = 0;
	while (std::getline(in, line))
	{
		BOOST_CHECK_EQUAL(line, "Hello, world!");
		++n;
	}

	BOOST_CHECK_EQUAL(n, 100);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(t_1000)
{
	fs::path f = gTestDir / "hello-1000.txt.gz";

	gxrio::gzip::ifstream in(f, std::ios::in | std::ios::binary);

	std::string line;

	int n = 0;
	while (std::getline(in, line))
	{
		std::string test = "Hello, world! - this is line " + std::to_string(n);

		BOOST_CHECK_EQUAL(line, test);
		++n;
	}

	BOOST_CHECK_EQUAL(n, 1000);
}

// --------------------------------------------------------------------

// Copy an istream
BOOST_AUTO_TEST_CASE(t_copy_1)
{
	struct membuf : public std::streambuf
	{
		membuf(char *text, size_t length)
		{
			this->setg(text, text, text + length);
		}
	} buffer(reinterpret_cast<char *>(kGZippedData), sizeof(kGZippedData));

	gxrio::gzip::istream in(&buffer);

	gxrio::gzip::istream in2(std::move(in));

	std::string line;
	std::getline(in2, line);

	BOOST_CHECK_EQUAL(line, "Hello, world!");
}

BOOST_AUTO_TEST_CASE(t_copy_2)
{
	fs::path f = gTestDir / "hello-1000.txt.gz";

	gxrio::gzip::ifstream in_1(f, std::ios::in | std::ios::binary);

	std::string line;

	std::getline(in_1, line);
	BOOST_CHECK_EQUAL(line, "Hello, world! - this is line 0");

	gxrio::gzip::ifstream in_2(std::move(in_1));
	std::getline(in_2, line);
	BOOST_CHECK_EQUAL(line, "Hello, world! - this is line 1");

	gxrio::gzip::ifstream in_3;
	in_3 = std::move(in_2);
	std::getline(in_3, line);
	BOOST_CHECK_EQUAL(line, "Hello, world! - this is line 2");

}

// --------------------------------------------------------------------


template <std::size_t SIZE, class CharT = char>
class ArrayedStreamBuffer : public std::basic_streambuf<CharT>
{
  public:
	using Base = std::basic_streambuf<CharT>;
	using char_type = typename Base::char_type;
	using int_type = typename Base::int_type;

	ArrayedStreamBuffer()
		: buffer_{} // value-initialize buffer_ to all zeroes
	{
		Base::setp(buffer_.begin(), buffer_.end()); // set std::basic_streambuf
		                                            // put area pointers to work with 'buffer_'
	}

	void reading()
	{
		Base::setg(Base::pbase(), Base::pbase(), Base::pptr());
	}

	int_type overflow(int_type ch)
	{
		std::cout << "overflow\n";
		return Base::overflow(ch);
	}

  private:
	std::array<char_type, SIZE> buffer_;
};

BOOST_AUTO_TEST_CASE(d_1)
{
	ArrayedStreamBuffer<100> buffer;

	gxrio::gzip::basic_ogzip_streambuf<char, std::char_traits<char>> zb;
	zb.init(&buffer);

	zb.sputn("Hello, world!", 13);

	zb.close();

	buffer.reading();

	gxrio::gzip::istream in(&buffer);

	std::string line;
	std::getline(in, line);

	BOOST_CHECK_EQUAL(line, "Hello, world!");
}

BOOST_AUTO_TEST_CASE(d_2)
{
	auto filename = "hello-1000.txt.gz";

	fs::path in_file = gTestDir / filename;
	fs::path out_file = fs::temp_directory_path() / filename;

	gxrio::gzip::ifstream in;
	gxrio::gzip::ofstream out;

	in.open(in_file);
	out.open(out_file);

	BOOST_ASSERT(in.is_open());
	BOOST_ASSERT(out.is_open());

	std::string line;

	while (getline(in, line))
		out << line << std::endl;

	in.close();
	out.close();

	in.open(in_file);
	gxrio::gzip::ifstream in_test(out_file);

	BOOST_ASSERT(in.is_open());
	BOOST_ASSERT(in_test.is_open());

	int n = 0;
	std::string test_line;

	for (;;)
	{
		bool b1 = (bool)getline(in, line);
		bool b2 = (bool)getline(in_test, test_line);

		BOOST_CHECK(b1 == b2);

		if (not (b1 and b2))
			break;

		BOOST_CHECK_EQUAL(line, test_line);
		++n;
	}

	BOOST_CHECK_EQUAL(n, 1000);
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(d_3)
{
	ArrayedStreamBuffer<100> buffer;

	gxrio::gzip::basic_ogzip_streambuf<char, std::char_traits<char>> zb;
	zb.init(&buffer);

	zb.sputn("aap ", 4);

	gxrio::gzip::basic_ogzip_streambuf<char, std::char_traits<char>> zb2(std::move(zb));

	zb2.sputn("noot ", 5);

	gxrio::gzip::basic_ogzip_streambuf<char, std::char_traits<char>> zb3;

	zb3 = std::move(zb2);

	zb3.sputn("mies\n", 5);

	zb3.close();

	buffer.reading();

	gxrio::gzip::istream in(&buffer);

	std::string line;
	std::getline(in, line);

	BOOST_CHECK_EQUAL(line, "aap noot mies");
}

