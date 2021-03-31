class PERSON{

private:



public:
	int id;
	int conf;
	//unsigned char *id;

	PERSON(){
		conf=0;
		id=0;
	}
	~PERSON(){

	}

	int setid(unsigned char *);
	//int getid(unsigned char *uID);
	int setconf(int);
	unsigned char* getid();
	int getconf();
	int plusconf(int a);

};


