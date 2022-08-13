About gxrio
===========

For years I've been writing code that used _boost::iostreams_ only to transparently support
gzipped files as input. But the dependency on _boost::iostreams_ proved to be a source of
confusion and errors. Mostly due to the fact that using statically linked boost libraries
also need to link to _zlib_.

And like I said, I only need the _gzip_ functionality. So I took a day off to write my own
stream classes. But when I was done, I realised _gzip_ is perhaps no longer hippest kid in
town and _xz_ looked like a nice alternative.
Fortunately, the API was rather similar and so I created both implementations.

To make life easier, there now is a generic `gxrio::istream` class that can read files
compressed with _gzip_ and _xz_ but also uncompressed files. It does this by inspecting
the first few bytes of an input stream (the signature) and decide what algorith to use
based on that.

Installation
------------

This library is a header only library. No need to compile anything. Unless you want to run
the unit-tests of course.

So building and installing is a simple as:

```
cmake . (optionally providing e.g. -DCMAKE_INSTALL_PREFIX=$HOME/.local )
cmake --build .
cmake --install .
```

Usage
-----

The goal was to have simple replacements for `std::istream` and `std::ifstream` that read
from _gzipped_ files and deflate them on the fly, transparently. And of course the same
for the output variants.

Note that due to the nature of compressed files, there is no way to seek in opened streams.
Allthough, if anyone can convince me I might add that since the zlib implementation seems
to have support for this.

Opening a file for reading is as simple as:

```
#include <iostream>
#include <gxrio.hpp>

int main(int argc, char * const argv[])
{
	gxrio::ifstream in(argv[1]);

	if (not in.is_open())
	{
		std::cerr << "Could not open file" << std::endl;
		exit(1);
	}

	std::string line;
	while (getline(in, line))
		std::cout << line << std::endl;

	return 0;
}
```

This will work on regular files, files compressed with _gzip_ and file compressed with _xz_.

You can also use another buffer as input, as in this example:

```
	unsigned char kGZippedData[] = {
		0x1f, 0x8b, 0x08, 0x08, 0x61, 0xb2, 0xf0, 0x62, 0x00, 0x03, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74,
		0x78, 0x74, 0x00, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0xd7, 0x51, 0x28, 0xcf, 0x2f, 0xca, 0x49, 0x51,
		0xe4, 0x02, 0x00, 0x18, 0xa7, 0x55, 0x7b, 0x0e, 0x00, 0x00, 0x00
	};

	struct membuf : public std::streambuf
	{
		membuf(char *text, size_t length)
		{
			this->setg(text, text, text + length);
		}
	} buffer(reinterpret_cast<char *>(kGZippedData), sizeof(kGZippedData));

	gxrio::istream in(&buffer);

```

Writing is similar, however, having a _gxrio::ostream_ class is not very useful. It won't know what to
do. But a _gxrio::ofstream_ class is provided, it will decide what compression to use based on the
filename extension. So, writing becomes as simple as:

```
	gxrio::ofstream out("hello.xz");
	out << "Hello, world!" << std::endl;
	out.close();
```
