//
// Foo.h
//
#pragma once

class  CFoo
{
public:
	CFoo()
	{
		szString[0] = '\0';
	}
public:
	char	szString[256];	// 256 byte
};

struct FOO
{
	char	szString[256];	// 256 byte
};

CFoo* NewFoo();

FOO*  MallocFoo();
FOO*  CallocFoo();
