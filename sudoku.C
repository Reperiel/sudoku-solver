/*
 * inputformat:
 * _ = undefined value
 * [1-9] = fixed value
 * whitespace is ignored
 *
 * N*N values are being read
 */
#if 0 // example:
_ _ _ _ _ _ _ 6 9
_ _ 7 1 _ 8 _ _ _
_ _ 8 _ 6 _ 3 _ _
_ _ 2 _ 4 _ _ _ _
_ _ _ _ _ _ 5 _ 1
5 _ 3 7 _ _ 4 _ _
1 _ _ 8 2 6 _ _ _
_ _ 5 _ _ _ _ 9 _
7 _ _ 5 9 _ _ 2 _
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <algorithm>
#include <string>
#include <cstring>

using namespace std;

static const bool debug = false;

class Sudoku {
	static const int Q = 3;
	static const int N = Q*Q;
	static const int B = N;
	static const int all = (1 << B) -1;

	static int count; // how many tests are done

	int field[N][N];
	int indent = 0; // indent for debug

	static string m2z( int m ); // depiction of one field

public:
	Sudoku( int ind );

	bool prospect( int x, int y ); // exhaustive test of one field
	bool subCheck( int qx, int qy ); // check one sub-quadrat
	bool set( int x, int y, int num ); // set one field, adapt row & column

	bool valid(); // solution correkt?

	bool read();
	void print( int ind=0 );
};

int Sudoku::count = 0;

Sudoku::Sudoku(int ind )
: indent(ind)
{
	for (int x=0; x<N; ++x) {
		for (int y=0; y<N; ++y) {
			field[x][y] = all;
		}
	}
}

bool
Sudoku::prospect( int x, int y )
{
	if (debug) {
		printf( "prospect: x=%d, y=%d\n", x, y );
		print( indent );
	}

	++count;

	int cur = field[x][y];
	if (!cur) {
		printf( "empty field, x=%d, y=%d?\n", x, y );
		print();
		abort();
	}

	// next field
	int x2 = (x + 1) % N;
	int y2 = x2 ? y : y+1;

	int cnt = 0;
	for (int b=0; b<B; ++b) {
		int m = (1 << b);
		if (cur & m) { // number is possible
			++cnt;
			if (int rem = cur ^ m) { // other remaining possibilities
				if (x == N-1 && y == N-1) { // not unique at the end
					if (debug)
						printf( "[end of field]" );
					return false;
				}
				Sudoku test = *this;
				++test.indent;
				if (!test.set( x, y, b )) { // not possible wrt row/col
					if (debug)
						printf( "[fail %d]", b+1 );
					continue;
				}
				if (test.prospect( x2, y2 )) {
					// remaining matrix doesn't work out
					if (debug)
						printf( "[ok %d %d]", x2, y2 );
					memcpy( field, test.field, sizeof(field) );
					return true;
				}
			}
			else {
				if (!set( x, y, b )) {
					// is this possible?
					// late detection of failed contraint
					if (debug)
						printf( "[late fail %d]", b+1 );
					return false;
				}
			}
		}
	}
	if (cnt > 1) // nothing found or not unique
		return false;
	if (x == N-1 && y == N-1) { // search completed without error
		if (debug)
			printf( "[end of field, ok]" );
		return true;
	}
	++indent;
	bool ret = prospect( x2, y2 ); // field was already definite, next one
	--indent;
	return ret;
}

bool
Sudoku::subCheck( int qx, int qy )
{
	int xoff = qx * Q;
	int yoff = qy * Q;
	int m = 0;
	for (int x=0; x<Q; ++x) {
		for (int y=0; y<Q; ++y) {
			m |= field[xoff+x][yoff+y];
		}
	}
	if ((m&all) != all) { // something missing
		if (debug) {
			printf( "[subCheck( %d, %d, %s ) failed]"
				,qx, qy, m2z(m&all).c_str() );
		}
		return false;
	}
	return true;
}

bool
Sudoku::set( int x, int y, int bit )
{
	bool ret = true;

	bool chk[Q][Q]; // which sub-quadrants have to be checked
	for (int qx=0; qx<Q; ++qx) {
		for (int qy=0; qy<Q; ++qy) {
			chk[qx][qy] = false;
		}
	}
	chk[x/Q][y/Q] = true;

	int m = (1 << bit);
	int invm = ~m;
	field[x][y] = m;

	// adapt row
	for (int x2=0; x2<N; ++x2) {
		if (x2 != x) {
			int cur = field[x2][y];
			int mcur = cur & invm;
			if (!mcur) { // not possible
				ret = false;
				if (!debug)
					return ret;
			}
			if (mcur != cur) { // changed
				chk[x2/Q][y/Q] = true;
				field[x2][y] = mcur;
			}
		}
	}

	// adapt column
	for (int y2=0; y2<N; ++y2) {
		if (y2 != y) {
			int cur = field[x][y2];
			int mcur = cur & invm;
			if (!mcur) { // not possible
				ret = false;
				if (!debug)
					return ret;
			}
			if (mcur != cur) { // changed
				chk[x/Q][y2/Q] = true;
				field[x][y2] = mcur;
			}
		}
	}

	// check sub-quadrants
	for (int qx=0; qx<Q; ++qx) {
		for (int qy=0; qy<Q; ++qy) {
			if (chk[qx][qy]) {
				if (!subCheck( qx, qy )) {
					ret = false;
					if (!debug)
						return ret;
				}
			}
		}
	}

	if (debug) {
		printf( "set( %d, %d, %x ): indent=%d, ret=%d\n"
			,x, y, bit+1, indent, ret );
		print();
	}

	return ret;
}

bool
Sudoku::valid()
{
	for (int x=0; x<N; ++x) {
		int m = 0;
		for (int y=0; y<N; ++y) {
			int cur = field[x][y];
			if (m & cur)
				return false;
			m |= cur;
		}
		if (m != all)
			return false;
	}
	for (int y=0; y<N; ++y) {
		int m = 0;
		for (int x=0; x<N; ++x) {
			int cur = field[x][y];
			if (m & cur)
				return false;
			m |= cur;
		}
		if (m != all)
			return false;
	}
	for (int qx=0; qx<Q; ++qx) {
		for (int qy=0; qy<Q; ++qy) {
			if (!subCheck( qx, qy ))
				return false;
		}
	}
	return true;
}

bool
Sudoku::read()
{
	Sudoku raw(0);
	for (int y=0; y<N; ++y) {
		for (int x=0; x<N; ++x) {
			int ch;
			do {
				ch = getchar();
				if (ch == EOF)
					return false;
			} while (isspace( ch ));
			if (ch == '_')
				continue;
			else if (isdigit(ch)) {
				int b = ch - '1';
				if (!set( x, y, b ))
					return false;
				raw.field[x][y] = (1<<b);
			}
			else
				return false;
		}
	}
	if (true || debug) {
		printf( "read:\n" );
		raw.print();
	}
	return true;
}

void
Sudoku::print( int ind )
{
	ind += indent + 1;
	if (count || indent)
		printf( "count=%d, indent=%d\n", count, indent );
	int len = 1;
	for (int y=0; y<N; ++y) {
		for (int x=0; x<N; ++x) {
			int cur = field[x][y];
			int clen = m2z( cur ).size();
			len = max( len, clen );
		}
	}

	for (int y=0; y<N; ++y) {
		if (y) {
			char line = y%Q ? '-' : '+';
			printf( "%*s", indent*2, "" );
			for (int x=0; x<N; ++x) {
				if (x)
					putchar ('+' );
				for (int i=0; i<len+1+2*(!!x); ++i)
					putchar( line );
			}
			putchar( '\n' );
		}
		printf( "%*s", indent*2, "" );
		for (int x=0; x<N; ++x) {
			if (x)
				printf( " %c| ", (x%Q ? ' ' : '|') );
			int cur = field[x][y];
			string s = m2z( cur );
			printf( "%-*s", len, s.c_str() );
		}
		putchar( '\n' );
	}
	putchar( '\n' );
}

string
Sudoku::m2z( int m )
{
	string ret;
	if (!m)
		ret.push_back( 'X' );
	else if (m == all)
		ret.push_back( '_' );
	else {
		for (int b=0; b<B; ++b) {
			if (m & (1 << b))
				ret.push_back( b + '1' );
		}
	}
	return ret;
}

int
main()
{
	Sudoku test(0);

	if (!test.read()) {
		printf( "read error\n" );
		if (debug)
			test.print();
		exit( EXIT_FAILURE );
	}
	if (debug)
		test.print();

	if (test.prospect( 0, 0 )) { // search from start
		if (!test.valid())
			abort();
		test.print();
	}
	else {
		printf( "no unique solution\n" );
		exit( EXIT_FAILURE );
	}

	exit( EXIT_SUCCESS );
}
