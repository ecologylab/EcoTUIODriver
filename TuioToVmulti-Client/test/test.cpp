// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include "iostream"
#include <fstream>
#include <iostream>
#include <string>
#include <atlbase.h>
#include <direct.h>

using namespace std;

int main(int argc, char* argv[])
{
	char *path = NULL;
path = getcwd(NULL, 0); // or _getcwd
if ( path != NULL)
    printf("%s\n", path);
	cout<<"rajat";
	string STRING;
	ifstream infile;
	infile.open ("../Data/tuioport5.txt");
    
	getline(infile,STRING); // Saves the line in STRING.
	cout<<"rajat2";
	
	infile.close();
	char *a=new char[STRING.size()+1];
	a[STRING.size()]=0;
	memcpy(a,STRING.c_str(),STRING.size());
	int port = atoi( a );
	int r =545654;
	cout<<endl;
	cout<<"port = "<<port<<endl;
	cout<<"dfsafd";

	getchar();

	return 0;
}

