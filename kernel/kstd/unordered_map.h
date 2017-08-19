#ifndef OLLIOS_GIT_UNORDERED_MAP
#define OLLIOS_GIT_UNORDERED_MAP

#include "functional.h"
#include "new.h"

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

		unordered_map(const unordered_map<K,V>& map)
		{
			copyFrom(map);
		}

		unordered_map& operator= (const unordered_map<K,V>& map)
		{
			clear();
			copyFrom(map);
			return *this;
			//return new unordered_map(map);
		}

		~unordered_map()
		{
			clear();
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
				{
					return _values[bucket][i];
				}
			}

			// None found, so we need to create it.
			size_t used = _used[bucket];
			if (used >= _bucketSize)
			{
				increaseSize();
				return (*this)[k];
			}
			else
			{
				new (&_keys[bucket][used]) K(k);
				new (&_values[bucket][used]) V();
				_used[bucket]++;
				return _values[bucket][used];
			}
		}

		size_t erase(const K& k)
		{
			size_t hash = std::hash<K>()(k);
			size_t bucket = hash % _numBuckets;

			for (size_t i = 0; i < _used[bucket]; i++)
			{
				if (k == _keys[bucket][i])
				{
					_values[bucket][i].~V();
					_keys[bucket][i].~K();
					for (size_t l = i; l < _used[bucket]-1; l++)
					{
						_keys[bucket][l] = _keys[bucket][l+1];
						_values[bucket][l] = _values[bucket][l+1];
					}
					_used[bucket]--;
					return 1;
				}
			}

			return 0;
		}

		size_t count(const K& k)
		{
			size_t hash = std::hash<K>()(k);
			size_t bucket = hash % _numBuckets;

			for (size_t i = 0; i < _used[bucket]; i++)
			{
				if (k == _keys[bucket][i])
				{
					return 1;
				}
			}
			return 0;
		}

	private:
		void clear()
		{
			for (size_t bucket = 0; bucket < _numBuckets; bucket++)
			{
				for (size_t i = 0; i < _used[bucket]; i++)
				{
					_values[bucket][i].~V();
					_keys[bucket][i].~K();
				}

				delete[] _values[bucket];
				delete[] _keys[bucket];
			}
			delete[] _values;
			delete[] _keys;
			delete[] _used;

			_numBuckets = 16;
			_bucketSize = 8;
		}

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
				for (size_t i = 0; i < oldUsed[bucket]; i++)
				{
					oldValues[bucket][i].~V();
					oldKeys[bucket][i].~K();
				}

				delete[] oldValues[bucket];
				delete[] oldKeys[bucket];
			}
			delete[] oldValues;
			delete[] oldKeys;
			delete[] oldUsed;
		}

		void copyFrom(const unordered_map<K,V>& map)
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
	};
}

#endif