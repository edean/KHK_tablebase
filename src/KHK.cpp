//============================================================================
// Name        : KHK.cpp
// Author      : Edward T. Dean
// Description : Tablebase generator for KHK in Seirawan Chess
//============================================================================

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;


////////////////////////////////////////////////////////////////////////////////////////////////
// The stuff here involves how positions might ultimately be stored (and used, when
// accessing the tablebases) in a fuller development. But for the initial creation stage,
// I'm using more rudimentary storage. Once the tablebase is fully formed,
// I may convert the KHK.0, KHK.1, ..., KHK.draw files to a format like this.

// Constants for the various chess pieces.
const signed char Blank = 0;
const signed char WK = 1;
const signed char BK = -1;
const signed char WE = 7;

class position
{
public:
	signed char board_array [128];
	bool white_to_move;
};
////////////////////////////////////////////////////////////////////////////////////////////////



// Constants to name the symmetries of the board.
const int sym_id = 1;
const int sym_x = 2; // reflect across x axis
const int sym_y = 3;
const int sym_d1 = 4; // reflect across a1-h8 diagonal
const int sym_d2 = 5;
const int sym_r90 = 6; // rotate 90 degrees clockwise
const int sym_r180 = 7;
const int sym_r270 = 8;





bool Is_Special_Square (int a)
{
	// These are the only squares (thanks to symmetry) we'll consider possible for the white king.
	if ((a==0) || (a==1) || (a==2) || (a==3) ||
			(a==17) || (a==18) || (a==19) || (a==34) || (a==35) || (a==51))
		return true;
	else
		return false;
}



int Apply_Symmetry (int sym, int square)
{
	// Applies the specified symmetry of the board to the specified square (in 0x88 terms).

	if (sym == sym_d1) {
		return (square / 16) + ((square % 16) * 16);
	}

	else if (sym == sym_y) {
		return ((square / 16) * 16) + (7 - (square % 16));
	}

	else if (sym == sym_d2) {
		return (7 - (square / 16)) + ((7 - (square % 16)) * 16);
	}

	else if (sym == sym_x) {
		return (square % 16) + ((7 - (square / 16)) * 16);
	}

	else
		return 0;
}



bool Hawk_Attacks(int a, int b, int c)
{
	// With white king on a, does a Hawk on square b attack square c?

	// First see if it attacks it like a knight.
	if ((c == b-33) || (c == b-31) || (c == b-18) || (c == b-14) ||
			(c == b+14) || (c == b+18) || (c == b+31) || (c == b+33))
		return true;
	// Now see if b and c are on the same up-right diagonal, without a in between.
	else if (((c-b)%17)==0 && (c != b) &&
			!((((c-a)%17)==0) && (b<a) && (a<c)) &&
			!((((c-a)%17)==0) && (c<a) && (a<b)))
		return true;
	// Now see if b and c are on the same up-left diagonal, without a in between.
	else if (((c-b)%15)==0 && (c != b) &&
			!((((c-a)%15)==0) && (b<a) && (a<c)) &&
			!((((c-a)%15)==0) && (c<a) && (a<b)))
		return true;
	else
		return false;
}


bool On_Board (int a)
{
	if ((a & 0x88) == 0)
		return true;
	else
		return false;
}


bool Adjacent (int a, int b)
{
	// Test to see if squares a and b on the board are adjacent.

	return ((b == a+1) || (b == a-1) || (b == a-17) || (b == a-16) || (b == a-15)
			|| (b == a+15) || (b == a+16) || (b == a+17));
}



bool Black_Is_Mated (int i, int j, int k, int white_on_move)
{
	// Given a KHK position (WK on i, BK on j, WH on k), determine whether black is in mate.

	if (white_on_move)
		return false;

	else {
		int c;

		for (int a=-1; a<=1; a++) {
			for (int b=-1; b<=1; b++) {
				c = j + (b * 16) + a;
				if (On_Board(c) && !(Hawk_Attacks(i,k,c) || Adjacent(i,c)))
					return false;
			}
		}

		return true;
	}
}


bool Is_Stalemate (int i, int j, int k, int white_on_move)
{
	// Given a KHK position (WK on i, BK on j, WH on k), determine whether black is stalemated.

	if (white_on_move)
		return false;

	else {
		int c;

		for (int a=-1; a<=1; a++) {
			for (int b=-1; b<=1; b++) {
				c = j + (b * 16) + a;
				if (On_Board(c) && (c != j) && !(Hawk_Attacks(i,k,c) || Adjacent(i,c)))
					return false;
			}
		}

		if (Hawk_Attacks(i,k,j))
			return false;
		else
			return true;
	}
}




void Create_KHK_Positions ()
{
	// Writes a list of all KHK mate positions (41 of them) to the file "KHK.0".
	// Writes a list of all remaining KHK positions (59,404 of them) to the file "KHK.pos", for further tablebase classification.
	// Each position given by: white_king_square black_king_square white_Hawk_square white_to_move \n (squares as in 0x88 board rep)
	// Works as advertised.

	ofstream pos_file ( "KHK.pos" );
	ofstream mate_file ( "KHK.0" );

	int cnt = 0;


	for (int i=0x00; i<0x78; i++) {

		if (On_Board(i) && Is_Special_Square(i)) {

			for (int j=0x00; j<0x78; j++) {

				if (On_Board(j) && (j != i) && (Adjacent(i,j)==false)) {

					for (int k=0x00; k<0x78; k++) {

						if (On_Board(k) && (k != i) && (k != j)) {

							// Include the position (either as mate or not), with black to move.
							if (Black_Is_Mated(i,j,k,0))
								mate_file << i << " " << j << " " << k << " " << false << "\n";
							else {
								pos_file << i << " " << j << " " << k << " " << false << "\n";
								cnt = cnt + 1;
							}


							// So long as the black king isn't attacked, include the position with white to move as well.
							if (Hawk_Attacks(i,k,j) == false) {
								pos_file << i << " " << j << " " << k << " " << true << "\n";
								cnt = cnt + 1;
							}
						}
					}
				}
			}
		}
	}
	// cout << cnt;
	pos_file.close();
	mate_file.close();
}


void White_One_Ply_More (int m)
{
	ifstream current_file ( "KHK.pos" );
	char board[256];
	int i, j, k, a;
	bool good_pos = false;

	string file_pre = "KHK.";
	string file_old, file_new;
	stringstream s_old, s_new;
	s_old << (m-1);
	s_new << m;
	file_old = file_pre + s_old.str();
	file_new = file_pre + s_new.str();


	ofstream out_file ( file_new.c_str() );
	ofstream new_file ( "KHK_alt.pos" );


	while (!current_file.eof()) {
		good_pos = false;

		current_file.getline(board,256);

		stringstream ss_one(board);
		ss_one >> i >> j >> k >> a;


		if (a == 0)
			new_file << i << " " << j << " " << k << " " << a << "\n";
		else {
			char tester [256];
			int x, y, z, w;

			for (int b=0x00; b<0x78; b++) {
				if (!(good_pos)) {
					if (On_Board(b) && Hawk_Attacks(i,k,b) && (b != i) && (b != k) && (b != j)) {
						ifstream in_file ( file_old.c_str() );
						while (!in_file.eof()) {
							in_file.getline(tester,256);

							stringstream ss(tester);
							ss >> x >> y >> z >> w;

							if ((x == i) && (y == j) && (z == b) && (w == 0)) {
								out_file << i << " " << j << " " << k << " " << a << "\n";
								good_pos = true;
								break;
							}
						}
						in_file.close();
					}
				}
				else
					break;
			}

			if (!(good_pos)) {
				for (int b=-1; b<=1; b++) {
					for (int c=-1; c<=1; c++) {

						if (!(good_pos)) {
							int d = i + (b * 16) + c;

							if (On_Board(d) && (d != i) && (d != j) && (d != k) && !(Adjacent(d,j))) {
								if (Is_Special_Square(d)) {
									ifstream in_file ( file_old.c_str() );

									while (!in_file.eof()) {
										in_file.getline(tester,256);

										stringstream ss(tester);
										ss >> x >> y >> z >> w;

										if ((x == d) && (y == j) && (z == k) && (w == 0)) {
											out_file << i << " " << j << " " << k << " " << a << "\n";
											good_pos = true;
											break;
										}
									}

									in_file.close();
								}

								else {
									// Depending on which non-special square the white king is moving to, apply the right symmetry to the piece positions,
									// and check whether THAT position is in the file in question. If so, add the original position to the right file.
									if ((d == 4) || (d == 20) || (d == 36) || (d == 52)) {
										int p = Apply_Symmetry (sym_y, d);
										int q = Apply_Symmetry (sym_y, j);
										int r = Apply_Symmetry (sym_y, k);

										ifstream in_file ( file_old.c_str() );

										while (!in_file.eof()) {
											in_file.getline(tester,256);

											stringstream ss(tester);
											ss >> x >> y >> z >> w;

											if ((x == p) && (y == q) && (z == r) && (w == 0)) {
												out_file << i << " " << j << " " << k << " " << a << "\n";
												good_pos = true;
												break;
											}
										}

										in_file.close();

									}

									else if ((d == 16) || (d == 32) || (d == 33) || (d == 49) || (d == 50)) {
										int p = Apply_Symmetry (sym_d1, d);
										int q = Apply_Symmetry (sym_d1, j);
										int r = Apply_Symmetry (sym_d1, k);

										ifstream in_file ( file_old.c_str() );

										while (!in_file.eof()) {
											in_file.getline(tester,256);

											stringstream ss(tester);
											ss >> x >> y >> z >> w;

											if ((x == p) && (y == q) && (z == r) && (w == 0)) {
												out_file << i << " " << j << " " << k << " " << a << "\n";
												good_pos = true;
												break;
											}
										}

										in_file.close();
									}

									else if (d == 67) {
										int p = Apply_Symmetry (sym_x, d);
										int q = Apply_Symmetry (sym_x, j);
										int r = Apply_Symmetry (sym_x, k);

										ifstream in_file ( file_old.c_str() );

										while (!in_file.eof()) {
											in_file.getline(tester,256);

											stringstream ss(tester);
											ss >> x >> y >> z >> w;

											if ((x == p) && (y == q) && (z == r) && (w == 0)) {
												out_file << i << " " << j << " " << k << " " << a << "\n";
												good_pos = true;
												break;
											}
										}

										in_file.close();
									}

									else if (d == 68) {
										int p = Apply_Symmetry (sym_d2, d);
										int q = Apply_Symmetry (sym_d2, j);
										int r = Apply_Symmetry (sym_d2, k);

										ifstream in_file ( file_old.c_str() );

										while (!in_file.eof()) {
											in_file.getline(tester,256);

											stringstream ss(tester);
											ss >> x >> y >> z >> w;

											if ((x == p) && (y == q) && (z == r) && (w == 0)) {
												out_file << i << " " << j << " " << k << " " << a << "\n";
												good_pos = true;
												break;
											}
										}

										in_file.close();
									}

									else if (d == 66) {
										int p = Apply_Symmetry (sym_d1, Apply_Symmetry(sym_y, d));
										int q = Apply_Symmetry (sym_d1, Apply_Symmetry(sym_y, j));
										int r = Apply_Symmetry (sym_d1, Apply_Symmetry(sym_y, k));

										ifstream in_file ( file_old.c_str() );

										while (!in_file.eof()) {
											in_file.getline(tester,256);

											stringstream ss(tester);
											ss >> x >> y >> z >> w;

											if ((x == p) && (y == q) && (z == r) && (w == 0)) {
												out_file << i << " " << j << " " << k << " " << a << "\n";
												good_pos = true;
												break;
											}
										}

										in_file.close();
									}

								}
							}
						}
						else
							break;
					}
				}
			}

			if (!(good_pos))
				new_file << i << " " << j << " " << k << " " << a << "\n";
		}
	}

	current_file.close();
	out_file.close();
	new_file.close();
}


void Black_One_Ply_More (int m)
{
	ifstream current_file ( "KHK_alt.pos" );
	char board[256];
	int i, j, k, a;
	bool so_far_so_good = true;

	string file_pre = "KHK.";
	string file_new;
	stringstream s_new;
	s_new << m;
	file_new = file_pre + s_new.str();


	ofstream out_file ( file_new.c_str() );
	ofstream new_file ( "KHK.pos" );

	while (!(current_file.eof())) {

		so_far_so_good = true;

		current_file.getline(board,256);

		stringstream ss_one(board);
		ss_one >> i >> j >> k >> a;

		if (a == 1)
			so_far_so_good = false;
		else {
			char tester[256];
			int x, y, z, w;

			for (int b=-1; b<=1; b++) {
				if (so_far_so_good) {
					for (int c=-1; c<=1; c++) {

						if (so_far_so_good) {

							int d = j + (b * 16) + c;

							if (On_Board(d) && (d != j) && !(Adjacent(i,d)) && !(Hawk_Attacks(i,d,k))) {

								so_far_so_good = false; // Assume for the sake of contradiction.

								for (int p=1; p<=(m-1); p=p+2) {
									if (!(so_far_so_good)) {
										string file_old;
										stringstream s_old;
										s_old << p;
										file_old = file_pre + s_old.str();

										ifstream in_file ( file_old.c_str() );

										while (!in_file.eof()) {
											in_file.getline(tester,256);

											stringstream ss(tester);
											ss >> x >> y >> z >> w;

											if ((x == i) && (y == d) && (z == k) && (w == 1)) {
												so_far_so_good = true;
												break;
											}
										}

										in_file.close();
									}
								}
							}
						}
						else
							break;
					}
				}
				else
					break;
			}
		}

		if (so_far_so_good && !(Is_Stalemate(i,j,k,a)))
			out_file << i << " " << j << " " << k << " " << a << "\n";
		else
			new_file << i << " " << j << " " << k << " " << a << "\n";
	}

	out_file.close();
	new_file.close();
	current_file.close();
}



int main()
{
	//Create_KHK_Positions ();

	//White_One_Ply_More(35);
	//Black_One_Ply_More(2);

	/*for (int i=29; i<=34; i=i+2) {
		White_One_Ply_More(i);
		Black_One_Ply_More(i+1);
	}*/

	return 0;
}
