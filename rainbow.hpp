
#ifndef incl__rainbow
#define incl__rainbow




#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <utility>
#include <tuple>
#include <stdexcept>

using namespace std;

// FORWARD DECLARATIONS
class RainbowKey;
class RainbowValue;




/*
 * class representing a 3-byte message
 * size: 3 bytes
 */
class RainbowKey
{
public:
  unsigned char k[3];	// 3 bytes

  RainbowKey();
  RainbowKey(RainbowKey const& cpy);
  RainbowKey(unsigned char const* const key);

  virtual bool operator==(RainbowKey const& o) const;
  virtual bool operator!=(RainbowKey const& o) const;
  virtual RainbowKey& operator=(RainbowKey const& o);

  // creates the SHA1 sum of the RainbowKey, encapsulated in a RainbowValue
	virtual RainbowValue hash();
  // counts the number of hash operations performed during a given program run
	static unsigned int hashcounter;

	virtual void dbgPrint() const;
	virtual void dbgPrintln() const;
};

/*
 * class representing a 20-byte SHA1 hash of a 3-byte message
 * size: 20 bytes
 */
class RainbowValue
{
public:
  unsigned int v[5];	// 20 bytes

  RainbowValue();
	RainbowValue(RainbowValue const& cpy);
  RainbowValue(unsigned int const* const val);

  virtual bool operator==(RainbowValue const& o) const;
  virtual bool operator!=(RainbowValue const& o) const;
  virtual RainbowValue& operator=(RainbowValue const& o);

  /*
  * creates a new RainbowKey by selecting bytes from the RainbowValue
  * for each byte i of the RainbowKey to be created (which is 3 bytes long),
  *  byte i is created by taking byte i of the RainbowValue
  *  and adding ci to it, modulo 256
  * (ci is one of the parameters c0, c1, c2)
  */
  virtual RainbowKey reduce(unsigned int c0,
	                          unsigned int c1,
							              unsigned int c2);
	virtual RainbowKey reduce(tuple<int, int, int> cs);

	virtual void dbgPrint() const;
	virtual void dbgPrintln() const;
};


namespace std
{
	template<>
	class hash<RainbowValue>
	{
	public:
		/*
		 * implementation of FNV
		 * so that RainbowValue can be used in unordered_list
		 */
		size_t operator()(RainbowValue const& val) const
		{
			size_t h = 2166136261;
			for (int i=0; i<20; i++) {
				unsigned char oct = ((unsigned char*) val.v)[i];
				h = h ^ oct;
				h = h * 16777619;
			}
			return h;
		}
	};

	template<>
	class less<RainbowValue>
	{
	public:
		bool operator()(RainbowValue const& a, RainbowValue const& b) const
		{
			for (int i=0; i<5; i++) {
				if		(a.v[i] < b.v[i]) return true;
				else if	(a.v[i] > b.v[i]) return false;
			}

			// they are equal if we reach this point
			return false;
		}
	};
}


class RainbowTable
{
public:
  /*
  * O(1) access datastructure to store rainbow table
  * key:   chain end (a RainbowValue)
  * value: corresponding chain start (a RainbowKey)
  */
	unordered_map<RainbowValue, RainbowKey> rainbow_map;

  /*
  * auxillary datastructure, also to store rainbow table
  * stores a list of 3-tuples
  * 1st element: chain start (RainbowKey)
  * 2nd element: RainbowKey that hashes directly to the chain end
  * 3rd element: chain end (RainbowValue)
  */
	vector<tuple<RainbowKey, RainbowKey, RainbowValue>> rainbow_list;

  /*
  * list containing numbers of colliding entries discarded
  * element i is the number of colliding entries encountered
  *  (and discarded) between entries (i-1) and i in rainbow_list
  * useful to reduce the size of the table by not storing chain starts
  *  while also removing collisions
  */
	vector<unsigned char> collisions;

  /*
   * list of words generated by the generateWords method
   */
	vector<RainbowKey> autogen_words;

  /*
   * sequence of reduce functions to be applied in the rainbow table
   * this should be fixed for every table instance
   */
	vector<tuple<int, int, int>> reduce_seq;

	RainbowTable(vector<tuple<int, int, int>> const& reduce_seq);
	RainbowTable();
  virtual ~RainbowTable();

  virtual void read(string filename);
	virtual void read(string filename, string collisionfilename);
	virtual void write(string filename, string collisionfilename);
	virtual void write(string filename);

	/*
	 * takes a starting key, k0
	 * assuming this->reduce_seq = r0,r1,...,r(k-1) and
	 * for all i: ki -> vi = h(ki) -> k(i+1) = ri(vi)
	 * return (k0, vk)
	 */
	virtual pair<RainbowKey, RainbowValue>
			computeChain(RainbowKey k0) const;
	/*
	 * same thing as above, but also yields the penultimate key
	 * after the last reduce, before hashing to the final value
	 */
	virtual tuple<RainbowKey, RainbowKey, RainbowValue>
			computeChainPenultimate(RainbowKey k0) const;
	/*
	 * either takes in a list of start points, words
	 * or takes in nothing and builds the words according to a rule
	 * populates rainbow_list using reduce_seq
	 */
	virtual void buildTable(vector<RainbowKey> const& words);

  /*
   * the same as buildTable, but uses autogen_words instead of a parameter
   */
	virtual void buildTable();

  /*
   * generates starting words (each a RainbowKey) for the
   * rainbow table in a fixed, deterministic manner
   * (deterministic => hardcoded)
   */
	virtual void generateWords();

	/*
	 * takes a hashed value, v
	 * checks if v is one of the terminating hashes
	 * returns true and the corresponding starting key if so
	 * returns false and some random key otherwise
	 */
	virtual pair<bool, RainbowKey>
			getChainStart(RainbowValue v) const;
	/*
	 * takes a hash, v
	 * takes a starting key, start
	 * traverses the hash chain to find the key that hashes to v
	 * returns that key
	 * ASSUMPTION: such a key exists
	 */
	virtual pair<bool, RainbowKey>
			getInverseInChain(RainbowValue v, RainbowKey start) const;
	/*
	 * takes a hash, v
	 * finds a chain to start from and traverses the chain
	 * to find the key that hashes to v
	 * returns true and the key if found
	 * returns false and a random key otherwise (FAILURE)
	 *
	 * THIS IS THE WHOLE POINT OF THE RAINBOW TABLE
	 */
	virtual pair<bool, RainbowKey>
			getInverse(RainbowValue v) const;
};




#endif
