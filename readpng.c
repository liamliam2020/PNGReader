// B. Liam Rethore, CS0449, Project 1, 10/5/2019

// Did part of the extra credit but didnt have time to finish it
// so the add fucntion will not actually add anything. 
// I also totally didnt see these given functions right below this
// untill I had made my own (expect for bswap32), so I didnt actaully ever call them.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// read the assignment instructions about this!
#define MAX_REASONABLE_TEXT_CHUNK_SIZE 1024

// the PNG file signature.
const char PNG_SIGNATURE[8] = {137, 'P', 'N', 'G', '\r', '\n', 26, '\n'};

// see if two strings are equal.
bool streq(const char* a, const char* b) { return strcmp(a, b) == 0; }

// see if the first n characters of two strings are equal.
bool strneq(const char* a, const char* b, int n) { return strncmp(a, b, n) == 0; }

// given a chunk's identifier and a type, sees if they're equal. use like:
//    <read a chunk's length and identifier>
//    if(is_chunk(identifier, "IHDR"))
bool is_chunk(const char* identifier, const char* type) {
	return strneq(identifier, type, 4);
}

// byte-swaps a 32-bit integer.
unsigned int bswap32(unsigned int x) {
	return
		((x & 0xFF) << 24) |
		((x & 0xFF00) << 8) |
		((x & 0xFF0000) >> 8) |
		((x & 0xFF000000) >> 24);
}

// ------------------------------------------------------------------------------------------------
// Commands (this is what you'll implement!!!!)
// ------------------------------------------------------------------------------------------------

typedef struct chunkHeader {  
		unsigned int length;
		char identifier[4];
	} chunkHeader;

typedef struct chunksPostHeader {
		unsigned int width;
		unsigned int height;
		char bitDepth;
		char colorType;
		char compression;
		char filter;
		char interlaced;
	} chunksPostHeader;

// Check to see if the given file is a .PNG
void chkPNG(FILE* readFrom) {
	char fileSign[8];
	char PNGsign[8] = {137, 'P', 'N', 'G', '\r', '\n', 26, '\n'}; 

	fseek(readFrom, 0, SEEK_SET); // Read 1st 8 bytes
	fread(&fileSign, sizeof(fileSign), 1, readFrom);
	fseek(readFrom, 0, SEEK_SET); 

	if (!strneq(fileSign, PNGsign, 2)) { // Error message and exit
		printf("File is not a PNG\n");
		exit(1);
	}
}

// Open a given file, check for if it exists, and
// check for proper file format
FILE* openFile(const char* fileName) {
	FILE* readFrom = fopen(fileName, "rb+");

	if (readFrom == NULL) { // Error messgae and exit
		printf("I couldn't find that file!\n");
		exit(1);
	}

	chkPNG(readFrom);

	return readFrom;
}

// Read the header of a given file starting at given offset
chunkHeader readHeader(FILE* PNGfile, int offset) {
	chunkHeader header;

	fseek(PNGfile, offset, SEEK_SET); // Set read to start at given location
	fread(&header, 8, 1, PNGfile);
	header.length = bswap32(header.length); // Switch since its big endian

	return header;
}

// Read all chunks after the header given a file
chunksPostHeader readPostHeader(FILE* PNGfile) {
	chunksPostHeader post;

	fread(&post, 13, 1, PNGfile); // 13 offset to skip magic number
	post.width = bswap32(post.width); // Switch since its big endian
	post.height = bswap32(post.height);

	return post;
}

// Read a given tEXt chunk 
void readText(FILE* PNGfile, chunkHeader header) {
	char *name = malloc(sizeof(char)*header.length); // Allocate size of header
	char *value;

	name[header.length] = '\0'; // Add zero terminator for reading 

	fread(name, header.length, 1, PNGfile);
	printf("%s:\n", name); // Only prints name since hits 1st zero terminator 
	value = &name[strlen(name) + 1]; // Pointer math to skip name that was just read
	printf("\t%s\n", value); 

	free(name); 
}

// Not implemented
void writeText(FILE* PNGfile) {
	printf("I Didn't have enought time to finish this :(\n");
}

// Show dimensions, bit depth, color type, and if interlaced
// for a given file
void show_info(const char* fileName) {
	FILE* PNGfile = openFile(fileName);

	chunkHeader header;
	chunksPostHeader post;
	header = readHeader(PNGfile, 8);
	post = readPostHeader(PNGfile);

	printf("File Info:\n");
	printf("\tDimensions: %u x %u\n", post.width, post.height);
	printf("\tBit Depth: %d\n", post.bitDepth);
	
	switch (post.colorType) {
		case 0:
			printf("\tColor Type: Grayscale\n");
			break;
		case 2:
			printf("\tColor Type: RGB\n");
			break;
		case 3:
			printf("\tColor Type: Indexed\n");
			break;
		case 4:
			printf("\tColor Type: Grayscale + Alpha\n");
			break;
		case 6:
			printf("\tColor Type: RGB + Alpha\n");
			break;	
	}

	if (post.interlaced == 0)
		printf("\tInterlaced: No\n");
	else
		printf("\tInterlaced: Yes\n");

	fclose(PNGfile);
}

// Show all chunk lengths and titles for a given file 
void dump_chunks(const char* fileName) {
	FILE* PNGfile = openFile(fileName);

	char endLoop[4] = {'I', 'E', 'N', 'D'}; // Last chunk name
	chunkHeader header;
	int offset = 8; // Start at 8 to skip length and identifer

	do { // Wil run untill last chunk name is found
		header = readHeader(PNGfile, offset);
		offset = offset + header.length + 12; // 12 offset for CRC, length, and identifer

		printf("'%.4s' (Length = %u)\n", header.identifier, header.length);
	} while (!strneq(header.identifier, endLoop, 2));
}

// Read the tExT chunks from a given file
void show_text(const char* fileName) {
	FILE* PNGfile = openFile(fileName);

	char endLoop[4] = {'I', 'E', 'N', 'D'}; // Last chunk name
	char checkChunk[4]= {'t', 'E', 'X', 't'};
 	chunkHeader header;
	int offset = 8; // Start at 8 to skip length and identifer

	do { // Wil run untill last chunk name is found
		header = readHeader(PNGfile, offset);
		offset = offset + header.length + 12; // 12 offset for CRC, length, and identifer

		if (strneq(header.identifier, checkChunk, 2)) { // if tEXt chunk is found then we read it
			readText(PNGfile, header);
		}
	} while (!strneq(header.identifier, endLoop, 2));

	fclose(PNGfile);
}

// This would add text but the writeText function is 
// not implemented so it doesnt work :(
void add_text(const char* fileName, const char* addName, const char* addValue) {
	FILE* PNGfile = openFile(fileName);

	if ((int)strlen(addName) > 79 || (int)strlen(addName) < 1) { // Error message and exit
		printf("ERROR:  Name of tExT chunk must be between 1-79\n");
		exit(1);
	}

	writeText(PNGfile); // Not implemented 

	fclose(PNGfile);
}

// ------------------------------------------------------------------------------------------------
// Argument parsing
// ------------------------------------------------------------------------------------------------

typedef enum {
	Info,
	DumpChunks,
	Text,
	Add,
} Mode;

typedef struct {
	const char* input;
	Mode mode;
} Arguments;

void show_usage_and_exit(const char* reason) {
	if(reason) {
		printf("Error: %s\n", reason);
	}

	printf("Usage:\n");
	printf("  ./pngedit input.png [command]\n");
	printf("Commands:\n");
	printf("  (no command)         displays PNG file info.\n");
	printf("  dump                 dump all chunks in the file.\n");
	printf("  text                 show all text chunks in the file.\n");
	printf("  add 'Name' 'Value'   add given name and value to tEXt chunk of a file");
	exit(1);
}

Arguments parse_arguments(int argc, const char** argv) {
	Arguments ret = {};

	switch(argc) {
		case 1: show_usage_and_exit(NULL);
		case 2: ret.mode = Info; break;
		case 3: {
			if(streq(argv[2], "dump")) {
				ret.mode = DumpChunks;
			} else if(streq(argv[2], "text")) {
				ret.mode = Text;
			} else {
				show_usage_and_exit("Invalid command.\n");
			}
			break;
		}
		case 5: ret.mode = Add; break;
		default: show_usage_and_exit("Invalid arguments.");
	}

	// if we get here, argv[1] is valid.
	ret.input = argv[1];
	return ret;
}

int main(int argc, const char** argv) {
	Arguments args = parse_arguments(argc, argv);

	switch(args.mode) {
		case Info:       show_info(args.input);   					break;
		case DumpChunks: dump_chunks(args.input); 					break;
		case Text:       show_text(args.input);   					break;
		case Add:		 add_text(args.input, argv[3], argv[4]);	break;
		default:
			printf("well this should never happen!\n");
			return 1;
	}

	return 0;
}