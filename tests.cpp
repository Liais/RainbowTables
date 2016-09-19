#include "rainbow.hpp"
#include "sha1/sha1.h"


bool io_test()
{
	RainbowTable tbl0;

	unsigned char k1_[3];
	k1_[0] = 'l';
	k1_[1] = 'o';
	k1_[2] = 'l';
	RainbowKey k1(k1_);

	unsigned int v1_[5];
	v1_[0] = 3;
	v1_[1] = 1;
	v1_[2] = 4;
	v1_[3] = 1;
	v1_[4] = 5;
	RainbowValue v1(v1_);

	unsigned char k2_[3];
	k2_[0] = 'w';
	k2_[1] = 't';
	k2_[2] = 'f';
	RainbowKey k2(k2_);

	unsigned int v2_[5];
	v2_[0] = 9;
	v2_[1] = 8;
	v2_[2] = 7;
	v2_[3] = 6;
	v2_[4] = 8;
	RainbowValue v2(v2_);

	unsigned char k3_[3];
	k3_[0] = 'c';
	k3_[1] = 'c';
	k3_[2] = 'b';
	RainbowKey k3(k3_);

	unsigned int v3_[5];
	v3_[0] = 1;
	v3_[1] = 2;
	v3_[2] = 7;
	v3_[3] = 4;
	v3_[4] = 5;
	RainbowValue v3(v3_);

	unsigned char k4_[3];
	k4_[0] = 'o';
	k4_[1] = 'm';
	k4_[2] = 'g';
	RainbowKey k4(k4_);

	unsigned int v4_[5];
	v4_[0] = 5;
	v4_[1] = 4;
	v4_[2] = 3;
	v4_[3] = 9;
	v4_[4] = 1;
	RainbowValue v4(v4_);

	unsigned char k5_[3];
	k5_[0] = 'b';
	k5_[1] = 'b';
	k5_[2] = 'q';
	RainbowKey k5(k5_);

	unsigned int v5_[5];
	v5_[0] = 100;
	v5_[1] = 102;
	v5_[2] = 104;
	v5_[3] = 106;
	v5_[4] = 108;
	RainbowValue v5(v5_);

	tbl0.rainbow_list.push_back(make_pair(k1, v1));
	tbl0.rainbow_list.push_back(make_pair(k2, v2));
	tbl0.rainbow_list.push_back(make_pair(k3, v3));
	tbl0.rainbow_list.push_back(make_pair(k4, v4));
	tbl0.rainbow_list.push_back(make_pair(k5, v5));

	tbl0.write("RAINBOW");


	RainbowTable tbl1;

	tbl1.read("RAINBOW");

	pair<RainbowKey, RainbowValue> p1 = tbl1.rainbow_list.at(0);
	pair<RainbowKey, RainbowValue> p2 = tbl1.rainbow_list.at(1);
	pair<RainbowKey, RainbowValue> p3 = tbl1.rainbow_list.at(2);
	pair<RainbowKey, RainbowValue> p4 = tbl1.rainbow_list.at(3);
	pair<RainbowKey, RainbowValue> p5 = tbl1.rainbow_list.at(4);
	for (int i=0; i<3; i++) printf("%c", p1.first.k[i]); printf(" | ");
	for (int i=0; i<5; i++) printf("%d ", p1.second.v[i]); printf("\n");
	for (int i=0; i<3; i++) printf("%c", p2.first.k[i]); printf(" | ");
	for (int i=0; i<5; i++) printf("%d ", p2.second.v[i]); printf("\n");
	for (int i=0; i<3; i++) printf("%c", p3.first.k[i]); printf(" | ");
	for (int i=0; i<5; i++) printf("%d ", p3.second.v[i]); printf("\n");
	for (int i=0; i<3; i++) printf("%c", p4.first.k[i]); printf(" | ");
	for (int i=0; i<5; i++) printf("%d ", p4.second.v[i]); printf("\n");
	for (int i=0; i<3; i++) printf("%c", p5.first.k[i]); printf(" | ");
	for (int i=0; i<5; i++) printf("%d ", p5.second.v[i]); printf("\n");

    return true;
}

bool hash_test()
{
	unsigned char testkey[3];
	unsigned int testhash[5];

	testkey[0] = 'o';
	testkey[1] = 'm';
	testkey[2] = 'g';

	SHA1 sha;
	sha.Reset();
	sha.Input(testkey[0]); sha.Input(testkey[1]); sha.Input(testkey[2]);
	sha.Result(testhash);

	RainbowKey testkeywrapper(testkey);
	RainbowValue testhashwrapper(testhash);

    printf("HASH TEST BEGIN\n");
	printf("hypothesis\n");
	for (int i=0; i<3; i++) printf("%c", testkeywrapper.k[i]);
	printf(" | ");
	for (int i=0; i<5; i++) printf(" %d ", testhashwrapper.v[i]);
	printf("\n");

	printf("result\n");
	for (int i=0; i<3; i++) printf("%c", testkeywrapper.k[i]);
	printf(" | ");
	for (int i=0; i<5; i++) printf(" %d ", testkeywrapper.hash().v[i]);
	printf("\n");
    printf("HASH TEST END\n");

	if (testhashwrapper != testkeywrapper.hash()) return false;

    return true;
}

bool reduce_test()
{
    RainbowValue testval;

    // all fit in the first byte on a little-endian system
    testval.v[0] = 182; // byte 0   = 182
    testval.v[1] = 201; // byte 4   = 201;
    testval.v[2] = 69;  // byte 8   = 69
    testval.v[3] = 42;  // byte 12  = 42
    testval.v[4] = 32;  // byte 16  = 32

    RainbowKey testkey;
    
    testkey.k[0] = (unsigned char) 201;
    testkey.k[1] = (unsigned char) 69;
    testkey.k[2] = (unsigned char) 32;

    printf("REDUCE TEST BEGIN\n");
    printf("hypothesis: "); for (int i=0; i<3; i++) printf("%d ", testkey.k[i]);                printf("\n");
    printf("result: ");     for (int i=0; i<3; i++) printf("%d ", testval.reduce(4,8,16).k[i]); printf("\n");
    printf("REDUCE TEST END\n");

    if (testkey != testval.reduce(4,8,16)) return false; // 4 8 16

    return true;
}


int main()
{
	map<string, bool> test_results;

    test_results["Hash Test"]       = hash_test();
    test_results["Reduce Test"]     = reduce_test();

    printf("\n\n");
    for (auto const& test_result : test_results)
        printf("%s: %s\n", test_result.first.c_str(), test_result.second ? "Passed" : "Failed");
}
