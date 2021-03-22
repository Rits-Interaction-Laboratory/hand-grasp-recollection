#include "stdafx.h"

#include "test.h"
#include "Foo.h"
#include <iostream>
//common
#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;
void test(){
	CFoo* pFoo_new = NewFoo();
	cout << "test" << endl;
	CFoo* pFoo_new2 = new CFoo;
}