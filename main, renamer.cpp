/*	----	----	----	----

// Programmer: Nicholas Prado
// created 10 11 20
	updated 10 12 2 now I see that I need to do the arrays dynamically so it doesn't keep asking for consts, pointers away
				10 12 11 changing all these chArrays to strings; need to delimit them properly and move to renaming files
				10 12 12 alright this works as intended. fix the coutput and the underscore management and this is ready for sharing
				11 6 7 cout only shows dA files, underscores become a single space except the edges

// Description: renames pictures from deviantart specifically, as denoted by the string '_by_' it will garble any with that otherwise
	
	3 options for file management: boost::filesystem dll, dirent.h or the afx.h I tried at first, and what I'm going to use: winapi
	http://msdn.microsoft.com/en-us/library/aa364418.aspx

	----	----	----	---- */
	
#include<iostream>
#include<fstream>
#include<string>
#include <tchar.h>
#include<stdio.h> // for rename
#include <windows.h> // pretty sure this gives me folder i/o
using namespace std;

void renameIfDAfile( TCHAR itsName[ ] );
	string::size_type deviantartFile( string theFilesName, string::size_type size );
	string craftSensibleName( string theFilesName, string::size_type fnSize, string::size_type titleSize );
		int replaceUnderscore( string &theFilesName, string::size_type titleSize );
			void shiftCharsLeft( string &theFilesName, int whereItStopped, string::size_type titleSize );
		string::size_type swapArtist_Title( string &name, string::size_type titleSize, int realTitle );
		void moveExtensionForward( string &name, string::size_type startHere, string::size_type extension );


int main( void ) // ready for file interaction -101211 // may need to make a copy of the name to preserve the original for renamer( )
{
	// perhaps I'll abstract this into another function and pass back a pointer of it or something
	TCHAR buffer4Name[ MAX_PATH ];
	DWORD dwRet;
	dwRet = GetCurrentDirectory( MAX_PATH, buffer4Name );
	if( dwRet == 0 || dwRet > MAX_PATH )
	{
		cout << "\nGetCurrentDirectory failed, let's end this so you can move it up the chain & use less chars";
		return 1;
	}
	cout << buffer4Name; // need to add \\*
	strcat( buffer4Name, "\\*\0" );

	HANDLE hFind;
	WIN32_FIND_DATA FindData;
	int ErrorCode;
	bool findingOk = true;

	hFind = FindFirstFile( buffer4Name, &FindData );

	if( hFind == INVALID_HANDLE_VALUE )
	{
		ErrorCode = GetLastError( );
		if ( ErrorCode == ERROR_FILE_NOT_FOUND )
		{
			cout << "There are no files matching that path/mask\n" << endl;
		}
		else
		{
			cout << "FindFirstFile( ) returned error code " << ErrorCode << endl;
		}
		findingOk = false;
	}
	else
	{
		renameIfDAfile( FindData.cFileName ); // first can't use findNextFile
	}

	if ( findingOk )
	{
		while ( FindNextFile( hFind, &FindData ) )
		{
			renameIfDAfile( FindData.cFileName );
		}

		ErrorCode = GetLastError( );
		if ( ErrorCode == ERROR_NO_MORE_FILES )
		{
			cout << endl << "\nAll files logged. Review & press Enter.";
			findingOk = false;
		}
		else
		{
			cout << "\nFindNextFile( ) returned error code " << ErrorCode;
		}

		if ( !FindClose( hFind ) ) // note below
		{
			ErrorCode = GetLastError( );
			cout << "\nFindClose( ) returned error code " << ErrorCode;
		}
	}
//	FindClose( hFind ); // his is causing the hang I see since the above performs FindClose in addition to reporting the state
	cin.ignore( 44, '\n' );

/*	real backend for these api's
typedef struct _WIN32_FIND_DATA {
	DWORD	dwFileAttributes;		FILETIME ftCreationTime, ftLastAccessTime, ftLastWriteTime;		DWORD	nFileSizeHigh, nFileSizeLow, dwReserved0,1; 
	TCHAR	cFileName[ MAX_PATH ]; cAlternateFileName[ 14 ]; 
} WIN32_FIND_DATA;
//	TCHAR :: typedef char TCHAR; // For Unicode platforms, TCHAR is defined as synonymous with the WCHAR type

		<< converting string into wchar
    string orig("Hello, World!");
    // Convert a wide char basic_string string to a multibyte char* string. To be safe, we allocate two bytes for each character in the original string, including the terminating null.
    const size_t newsize = (strlen(orig.c_str()) + 1)*2;
    char *nstring = new char[newsize];
    strcpy_s(nstring, newsize, orig.c_str());
    cout << nstring << " (char *)" << endl;
	
		<< converting wchar into string
	wchar_t *orig = _T("Hello, World!");
	// Convert the wide character wchar_t string to a basic_string, append the type of string to it, and display the result.
    wstring basicstring(orig);
    basicstring += _T(" (basic_string)");
    wcout << basicstring << endl;
*/

/* tests via strings in a file
	fstream testBed;
	testBed.open( "testing.txt", ios::in );
	if ( !testBed )
	{
		cout << "\n I'm missing the file. I should totally kill the process but I'll give you an opportunity to put it in right place";
		//giva an opportunity to or kill via shutDown
	}
	
	string theFilesName;
	int x = 7; //	<-- TESTING with actual number of data

	while ( x > 0 )
	{
		testBed >> theFilesName;
		string::size_type fnSize = theFilesName.length( );
		string::size_type titleSize = deviantartFile( theFilesName, fnSize );
		if ( titleSize != 0 )
	   {
			cout <<"\n\t" << theFilesName; // just for TESTING cut when done // pair below in craftSensibleName
			//cin.ignore( 99, '\n' );
			//rename( theFilesName , craftSensibleName( theFilesName, fnSize ) ); // chArrays; returns 0 when successful // do you mean chArrays exclusively?
			cout << "\nbecomes " << craftSensibleName( theFilesName, fnSize, titleSize ) << "\t( again? ) ";
			cin.ignore( 99 , '\n' );
			// as you can see, I'm outputting it to the screen, not changing the originals so don't
			// bother looking in the file, I can move to that later but primarily want to move to file renaming from here
	   }
		x--;
	}
	testBed.close( );
*/
	return 0;
}

void renameIfDAfile( TCHAR itsName[ ] )
{
	string theFilesName( itsName );
	string::size_type fileNameSize = theFilesName.length( );
	string::size_type titleSize = deviantartFile( theFilesName, fileNameSize );
	if ( titleSize != 0 )
	{
		cout << '\n' << theFilesName;
		theFilesName = craftSensibleName( theFilesName, fileNameSize, titleSize );
		rename( itsName , theFilesName.c_str( ) );
	}
}

// 11 6 7 consider researching C++'s regex so I don't need to do this or the next part so explicitly
// obviously this is a big assumption on my part and must be corrected if I decide to share this
// also It can't have by in the title like "Me, by the bay" without me changing it at least a little
string::size_type deviantartFile( string theFilesName, string::size_type size ) // seems ok	-101211
{
	string::size_type lett = 0;
	for ( ; lett < size; lett++)
	{
		// assuming that _by_ means it is a pic from da
		if ( theFilesName[ lett ] == '_' )
		{
			if ( theFilesName[ lett + 1 ] == 'b' ) // should I send it through lowerCaser? no. DA always uses lower case "_by_"
			{
				if ( theFilesName[ lett + 2 ] == 'y' ) // that is another means of distinguishing it's files from others
				{
					if ( theFilesName[ lett + 3 ] == '_' ) // I can probably get away with saying if token > # && == 'x'
					{
						return lett;
					}
				}
			}
		}
	}
	return 0; // ie not a dA file, hence sentinel
}

string craftSensibleName( string theFilesName, string::size_type fnSize, string::size_type titleSize ) // works 11 6 7
{
	int newTitleLength = replaceUnderscore( theFilesName, titleSize );
	
	int fullNameSize = swapArtist_Title( theFilesName, titleSize, newTitleLength );
	moveExtensionForward( theFilesName, fullNameSize, fnSize - 4 ); // always must be done because the title is 4 chars shorter by unreserving '_by_'
	
	return theFilesName;
}

int replaceUnderscore( string &theFilesName, string::size_type titleSize ) // works, leaves extra space when justDidIt 11 6 7
{
	int newTitleSize = titleSize;
	bool justDidIt = true; // to capture when the first char is an '_'
	theFilesName[ titleSize ] = 'x';
	for ( int nn = 0; nn < titleSize; nn++ )
	{
//cout << "\n nn = " << nn;
	if ( theFilesName[ nn ] == '_' )
		{
			if ( nn == newTitleSize - 1 )
			{
				--newTitleSize;
				break;
			}
			if ( justDidIt )
			{
				shiftCharsLeft( theFilesName, nn, titleSize );
				--nn; // since I just shifted them left, I'm looking at nn + 1
				--newTitleSize;
			}
			else
			{
				theFilesName[ nn ] = ' ';
				justDidIt = true;
			}
//				cout << "\ndoing it";
		}
		else
			justDidIt = false;
	}
	theFilesName[ titleSize ] = '_';
	return newTitleSize;
}

void shiftCharsLeft( string &theFilesName, int letterStoppedAt, string::size_type titleSize ) // works 11 6 7
{
	for ( int nn = letterStoppedAt; nn < titleSize; nn++ )
	{
		theFilesName[ nn ] = theFilesName[ nn + 1 ];
	}
	// used to cut off the last letter since I'm recopied the '_' from '_by_' since this overflows by grabbing at tSize
	//theFilesName[ titleSize - 1 ] = 'x';
}

string::size_type swapArtist_Title( string &theFilesName, string::size_type titleExpected, int realTitleSize ) // test again 11 6 7
{
	int whereItGoes = 0;
	string justTitle = theFilesName.substr( 0, realTitleSize );
	for ( string::size_type nn = titleExpected + 4; true; nn++ ) // either [name].jpg or newer have [name]-dw4rwe.png for some reason
	{
		theFilesName[ whereItGoes ] = theFilesName[ nn ];
		whereItGoes++;
		if ( theFilesName[ nn ] == '.' || theFilesName[ nn ] == '-' )
			break;
	}
	theFilesName[ --whereItGoes ] = ' '; // preincrement purposeful
	theFilesName.replace( ++whereItGoes, justTitle.length( ), justTitle );
	return whereItGoes + realTitleSize;
}


// moves the last four chars past garbage to the end of the new title; also, I changed the params so I am ready for reverse order
void moveExtensionForward( string &theFilesName, string::size_type startHere, string::size_type extension ) // okay	-101211
{
	for ( int uu = 4; uu > 0; uu-- ) // ie 4 times
	{
		theFilesName[ startHere ] = theFilesName[ extension ];
		++startHere;
		++extension;
	}
	theFilesName = theFilesName.substr( 0, startHere );
}















