#pragma once
template <typename M> void DeleteMap(M& map) {
	for (typename M::iterator it = map.begin(); it != map.end(); ++it) {
		delete it->second;
	}
	map.clear();
}

template <typename M> void DeleteVector(M& vector) {
	for (typename M::iterator it = vector.begin(); it != vector.end(); ++it) {
		delete (*it);
	}
	vector.clear();
}

#define SAFE_RELEASE(x)	if(x)	{ x->release(); x = nullptr;}
