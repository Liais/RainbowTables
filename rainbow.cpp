#include "rainbow.hpp"
#include "sha1/sha1.h"




RainbowKey::RainbowKey()
{
}
RainbowKey::RainbowKey(RainbowKey const& cpy)
{
	for (int i=0; i<3; i++)
		this->k[i] = cpy.k[i];
}
RainbowKey::RainbowKey(unsigned char const* const key)
{
    for (int i=0; i<3; i++)
        this->k[i] = key[i];
}

bool RainbowKey::operator==(RainbowKey const& o) const
{
    bool eq = true;
    for (int i=0; i<3; i++)
        eq = eq && (this->k[i] == o.k[i]);
    return eq;
}
bool RainbowKey::operator!=(RainbowKey const& o) const
{
    return !((*this) == o);
}
RainbowKey& RainbowKey::operator=(RainbowKey const& o)
{
    for (int i=0; i<3; i++)
        this->k[i] = o.k[i];
    return *this;
}

RainbowValue RainbowKey::hash()
{
	unsigned int hashed[5];

	SHA1 sha;
	sha.Reset();
	sha.Input(this->k[0]); sha.Input(this->k[1]); sha.Input(this->k[2]);
	sha.Result(hashed);
	// TODO: increment global SHA1 use count
	
	RainbowValue result(hashed);
	return result;
}

void RainbowKey::dbgPrint()
{
	for (int i=0; i<3; i++) printf("%c", this->k[i]);
}
void RainbowKey::dbgPrintln()
{
	this->dbgPrint();
	printf("\n");
}


RainbowValue::RainbowValue()
{
	for (int i=0; i<5; i++)
		this->v[i] = 6;
}
RainbowValue::RainbowValue(RainbowValue const& cpy)
{
	for (int i=0; i<5; i++)
		this->v[i] = cpy.v[i];
}
RainbowValue::RainbowValue(unsigned int const* const val)
{
    for (int i=0; i<5; i++)
        this->v[i] = val[i];
}

bool RainbowValue::operator==(RainbowValue const& o) const
{
    bool eq = true;
    for (int i=0; i<5; i++)
        eq = eq && (this->v[i] == o.v[i]);
    return eq;
}
bool RainbowValue::operator!=(RainbowValue const& o) const
{
    return !((*this) == o);
}
RainbowValue& RainbowValue::operator=(RainbowValue const& o)
{
    for (int i=0; i<5; i++)
        this->v[i] = o.v[i];
    return *this;
}

RainbowKey RainbowValue::reduce(unsigned int c0, unsigned int c1, unsigned int c2)
{
    assert(0 <= c0 && c0 < 20);
    assert(0 <= c1 && c1 < 20);
    assert(0 <= c2 && c2 < 20);

    unsigned char reduced[3];

    reduced[0] = *(((unsigned char*) this->v) + c0);
    reduced[1] = *(((unsigned char*) this->v) + c1);
    reduced[2] = *(((unsigned char*) this->v) + c2);

    RainbowKey result(reduced);
    return result;
}
RainbowKey RainbowValue::reduce(tuple<int, int, int> cs)
{
	return this->reduce(get<0>(cs), get<1>(cs), get<2>(cs));
}

void RainbowValue::dbgPrint()
{
	for (int i=0; i<5; i++) printf("%d ", this->v[i]);
}
void RainbowValue::dbgPrintln()
{
	this->dbgPrint();
	printf("\n");
}



RainbowTable::RainbowTable(vector<tuple<int, int, int>> reduce_seq_) : reduce_seq(reduce_seq_)
{
}

RainbowTable::~RainbowTable()
{
}

void RainbowTable::read(string filename)
{
	FILE* file_handle = nullptr;
	file_handle = fopen(filename.c_str(), "r");

	while (!feof(file_handle)) {
		RainbowKey   k;
		RainbowValue v;

		int r = fscanf(file_handle, "%c %c %c %u %u %u %u %u\n",
							k.k,
							k.k+1,
							k.k+2,
							v.v,
							v.v+1,
							v.v+2,
							v.v+3,
							v.v+4);
		if (r < 8) break;
		this->rainbow_list.push_back(make_pair(k, v));
	}

	fclose(file_handle);
}
void RainbowTable::write(string filename)
{
	FILE* file_handle = nullptr;
	file_handle = fopen(filename.c_str(), "w");

	for (auto const& entry : this->rainbow_list) {
		RainbowKey   k; k = entry.first.k;
		RainbowValue v; v = entry.second.v;

		fprintf(file_handle, "%c %c %c %u %u %u %u %u\n",
							k.k[0],
							k.k[1],
							k.k[2],
							v.v[0],
							v.v[1],
							v.v[2],
							v.v[3],
							v.v[4]);
	}

	fflush(file_handle);
	fclose(file_handle);
}

pair<RainbowKey, RainbowValue> RainbowTable::computeChain(RainbowKey k0)
{
	RainbowKey currkey = k0;
	RainbowValue currval;

	for (auto const& r : this->reduce_seq) {
		currval = currkey.hash();
		currkey = currval.reduce(r);
	}
	currval = currkey.hash();

	return make_pair(k0, currval);
}
void RainbowTable::buildTable(vector<RainbowKey> const& words)
{
	for (RainbowKey word : words)
		this->rainbow_list.push_back(this->computeChain(word));
}
pair<bool, RainbowKey> RainbowTable::getChainStart(RainbowValue v)
{
	for (auto const& p : this->rainbow_list) {
		if (p.second == v) return make_pair(true, p.first);
	}

	// not found, if we reach this point
	
	RainbowKey fake_key;
	return make_pair(false, fake_key);
}
RainbowKey RainbowTable::getInverseInChain(RainbowValue v, RainbowKey start)
{
	RainbowKey   key = start;
	RainbowValue val = start.hash();
	for (auto const& r : this->reduce_seq) {
		if (val == v) return key;

		key = val.reduce(r);
		val = key.hash();
	}

	assert (val == v);
	return key;
}
pair<bool, RainbowKey> RainbowTable::getInverse(RainbowValue v)
{
	int K = this->reduce_seq.size();

	for (int i=K; i>=0; i--) {
		RainbowValue hash_to_check = v;

		for (int j = i; j<K; j++) {
			hash_to_check = hash_to_check.reduce(this->reduce_seq[j]).hash();
		}

		auto chain_to_start_traversing = this->getChainStart(hash_to_check);
		if (chain_to_start_traversing.first) {
			// we have found a guaranteed starting point
			RainbowKey inv = this->getInverseInChain(v, chain_to_start_traversing.second);
			return make_pair(true, inv);
		}
	}
	// if we reach this point, FAIL
	RainbowKey fake_key;
	return make_pair(false, fake_key);
}

