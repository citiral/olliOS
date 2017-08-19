#ifndef OLLIOS_GIT_UNORDERED_MAP
#define OLLIOS_GIT_UNORDERED_MAP

#include "functional.h"

namespace std {
	template<class K, class V>
	class unordered_map {
	private:
		V** _values;
		K** _keys;
		size_t* _used;
		size_t _numBuckets = 16;
		size_t _bucketSize = 8;

	public:
		unordered_map()
		{
			allocateBuckets();
		}

		unordered_map(const unordered_map& map)
		{
			_numBuckets = map._numBuckets;
			_bucketSize = map._bucketSize;
			allocateBuckets();

			for (size_t bucket = 0; bucket < _numBuckets; bucket++)
			{
				for (size_t i = 0; i < _used[bucket]; i++)
				{
					new (&_values[bucket][i]) V(map._values[bucket][i]);
					new (&_keys[bucket][i]) K(map._keys[bucket][i]);
				}
			}
		}

		unordered_map& operator= (const unordered_map& map)
		{
			return new unordered_map(map);
		}

		~unordered_map()
		{
			for (size_t bucket = 0; bucket < _numBuckets; bucket++)
			{
				delete _values[bucket];
				delete _keys[bucket];
			}
			delete _values;
			delete _keys;
			delete _used;
		}

		/*V& at(const K& k)
		{
			size_t hash = std::hash<K>()(k);
			size_t bucket = hash % _numBuckets;

			for (size_t i = 0; i < _used[bucket]; i++)
			{
				if (k == _keys[bucket][i])
					return _values[bucket][i];
			}

			// Can't do this yet
			throws out_of_range;
		}*/

		V& operator[] (const K& k)
		{
			size_t hash = std::hash<K>()(k);
			size_t bucket = hash % _numBuckets;

			for (size_t i = 0; i < _used[bucket]; i++)
			{
				if (k == _keys[bucket][i])
					return _values[bucket][i];
			}

			// None found, so we need to create it.
			int used = _used[bucket];
			if (used >= _bucketSize)
			{
				increaseSize();
				return (*this)[k];
			}
			else
			{
				new (&_keys[bucket][used]) K(k);
				_used[bucket]++;
				return _values[bucket][used];
			}
		}

	private:
		void allocateBuckets()
		{
			_values = new V*[_numBuckets];
			_keys = new K*[_numBuckets];
			_used = new size_t[_numBuckets];

			for (size_t i = 0; i < _numBuckets; i++)
			{
				_values[i] = new V[_bucketSize];
				_keys[i] = new K[_bucketSize];
				_used[i] = 0;
			}
		}

		void put(const K& k, const V& v)
		{
			size_t hash = std::hash<K>()(k);
			size_t bucket = hash % _numBuckets;

			int used = _used[bucket];
			if (used >= _bucketSize)
			{
				increaseSize();
				put(k, v);
				return;
			}

			new (&_values[bucket][used]) V(v);
			new (&_keys[bucket][used]) K(k);
			_used[bucket]++;
		}

		// Increase the number of buckets by two and the number of positions in each bucket by two.
		// It will then copy the old values in the new map and delete the old values.
		void increaseSize()
		{
			V** oldValues = _values;
			K** oldKeys = _keys;
			size_t* oldUsed = _used;
			size_t oldNumBuckets = _numBuckets;
			size_t oldBucketSize = _bucketSize;
			
			_numBuckets *= 2;
			_bucketSize *= 2;

			allocateBuckets();

			for (size_t bucket = 0; bucket < oldNumBuckets; bucket++)
			{
				for (size_t i = 0; i < oldUsed[bucket]; i++)
				{
					K key = oldKeys[bucket][i];
					V value = oldValues[bucket][i];
					put(key, value);
				}
			}

			// Delete old map
			for (size_t bucket = 0; bucket < oldNumBuckets; bucket++)
			{
				delete oldValues[bucket];
				delete oldKeys[bucket];
			}
			delete oldValues;
			delete oldKeys;
			delete oldUsed;
		}
	};
}

#endif