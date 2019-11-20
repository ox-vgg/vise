(should be roughly around 2004?)
Code by Krystian Mikolajczyk

(should be roughly around 2007?)
Probably some bug fixes by James Philbin

2013
Memory leak fixes, slight cleanup, conversion to CMake by Relja Arandjelovic.
Note: there are MANY leaks around (e.g. vector<foo*> where foo's are never deleted), as well as programming errors like comparing pointers where the intention is to compare values. I focused on removing memory leaks as I didn't want to accidentally mess things up. I only fixed memory leaks in functions I use (hessian affine + SIFT) and a few bits found on the way.
