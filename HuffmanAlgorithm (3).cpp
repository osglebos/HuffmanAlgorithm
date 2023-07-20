#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <windows.h>
#define ALPHABET_SIZE 128
#define MAX_CODE 44

struct Letter
{
	int freq;
	char let;
	Letter *next;
};

struct CODE
{
	char *code;
	char let;
	CODE *next;
};

struct TREE
{
	double freq;
	char let;
	TREE *left;
	TREE *right;
};

#pragma region Check

int CheckSum(char *Filename)
{
	int sum = 0;
	int c = 0;
	FILE *File = fopen(Filename, "rt");

	if (File == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", Filename);
		return -1;
	}

	while ((c = getc(File)) != EOF) sum += c;

	fclose(File);
	return sum;
}

int Check3(char *Filename)
{
	int sum = 0;
	int c = 0;
	FILE *File = fopen(Filename, "rt");

	if (File == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", Filename);
		return -1;
	}

	while ((c = getc(File)) != EOF) sum++;

	fclose(File);
	return sum;
}

int Check(char *inFilename, char *outFilename)
{
	FILE *inFile = fopen(inFilename, "rt");
	FILE *outFile = fopen(outFilename, "rt");

	if (inFile == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", inFilename);
		return 0;
	}
	if (outFile == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", outFilename);
		return 0;
	}
	char fr, to;
	while (((fr = getc(inFile)) != EOF) && ((to = getc(outFile)) != EOF))
		if (fr != to)
			return 0;

	fclose(inFile);
	fclose(outFile);
	return 1;
}

#pragma endregion

#pragma region Lists

void AddLetter(Letter **L, double freq, int let)
{
	Letter *newElement = new Letter;
	if (newElement == 0)
	{
		fprintf(stderr, "Could not allocate %u bytes for the sovature\n", sizeof(Letter));
		return;
	}
	while (*L != NULL)
		if ((*L)->freq > freq)
			L = &((*L)->next);
		else
			break;
	newElement->let = let;
	newElement->freq = freq;
	newElement->next = (*L);
	*L = newElement;
}

void PushBack(Letter **L, double freq, int let)
{
	Letter *newElement;
	newElement = new Letter;
	if (newElement == 0)
	{
		fprintf(stderr, "Could not allocate %u bytes for the sovature\n", sizeof(Letter));
		return;
	}
	while (*L != NULL)
		L = &((*L)->next);

	newElement->let = let;
	newElement->freq = freq;
	newElement->next = NULL;
	*L = newElement;
}

char *FindCode(char c, CODE *Codes)
{
	for (CODE *i = Codes; i != NULL; i = i->next)
		if (c == i->let) return i->code;
	return Codes->code;
}

void AddCode(CODE **L, char let, char *str)
{
	CODE *newElement;
	newElement = new CODE;
	if (newElement == 0)
	{
		fprintf(stderr, "Could not allocate %u bytes for the sovature\n", sizeof(CODE));
		return;
	}

	newElement->let = let;
	newElement->code = _strdup(str);
	newElement->next = *L;
	*L = newElement;
}

#pragma endregion 

#pragma region Tree

TREE *Build(Letter *a)
{
	if (a != NULL)
	{
		TREE *head = new TREE;
		if (a->next == NULL)
		{
			head->freq = a->freq;
			head->left = NULL;
			head->right = NULL;
			head->let = a->let;
		}
		else
		{
			Letter *first = a;
			Letter *second = a;
			int count = 0;
			head->let = '-';
			head->freq = 0;
			for (Letter *j = a; j != NULL; j = j->next)	head->freq += j->freq;

			for (second = a; second->next->next != NULL; second = second->next)
			{
				double ffreq = 0, sfreq = 0;
				for (Letter *j = a; j != second->next->next; j = j->next) ffreq += j->freq;
				for (Letter *j = second->next->next; j != NULL; j = j->next) sfreq += j->freq;

				if (ffreq >= sfreq)	break;
			}

			Letter *tmp = second->next;
			second->next = NULL;
			second = tmp;

			head->left = Build(first);
			head->right = Build(second);
		}
		return head;
	}
	else return NULL;
}

void DisplayTurn(TREE *T, int dep)
{
	if (T != NULL)
	{
		if (T->right != NULL)
			DisplayTurn(T->right, dep + 1);

		for (int i = 0; i <= dep * 3; i++)printf(" ");
		printf("%1.5lf (%c)\n", T->freq, T->let);

		if (T->left != NULL)
			DisplayTurn(T->left, dep + 1);
	}
}
#pragma endregion

#pragma region Encode

void PrintCodesToFile(FILE *inF, FILE *outF, Letter *a, CODE *Codes)
{
	int c = 0;
	for (CODE *i = Codes; i != NULL; i = i->next) c++;
	fprintf(outF, "%i\n", c);

	for (Letter *i = a; i != NULL; i = i->next) if (i->freq != 0) fprintf(outF, "'%c' %lf\n", i->let, i->freq);

	fseek(inF, 0, SEEK_SET);
	while ((c = getc(inF)) != EOF)	fprintf(outF, "%s", FindCode(c, Codes));
	fseek(inF, 0, SEEK_SET);
}

Letter *CountFreq(FILE *F)
{
	Letter *letters = NULL;
	int freq[ALPHABET_SIZE] = { 0 };
	int c = 0;
	int sum = 0;

	while ((c = getc(F)) != EOF) {
		freq[c]++;
		sum++;
	}

	for (int i = 0; i < ALPHABET_SIZE; i++)
		if (freq[i] != 0)
			PushBack(&letters, round(freq[i] * MAX_POW / sum) / MAX_POW, i);
			
	fseek(F, 0, SEEK_SET);
	return letters;
}

void TraverseTREE(TREE *T, int deph, CODE **Codes, char *str)
{
	if (T != NULL)
	{
		if (!T->left || !T->right)
		{
			char *tmp_code = new char[deph + 1];
			for (int i = 0; i < deph + 1; i++) tmp_code[i] = str[i];
			tmp_code[deph] = 0;
			AddCode(Codes, T->let, tmp_code);
		}
		else
		{
			str[deph] = '1';
			if (T->right != NULL)
				TraverseTREE(T->right, deph + 1, Codes, str);
			str[deph] = '0';
			if (T->left != NULL)
				TraverseTREE(T->left, deph + 1, Codes, str);

		}
	}
}

int EncodeFunc(char *inFilename, char *outFilename) {
	Letter *letters, *letters_dup = NULL;
	TREE *root = NULL;
	CODE *Codes = NULL;
	FILE *inFile = fopen(inFilename, "rt");
	FILE *outFile = fopen(outFilename, "wt");

	if (inFile == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", inFilename);
		return 0;
	}
	if (outFile == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", outFilename);
		return 0;
	}

	letters = CountFreq(inFile);
	for (Letter *i = letters; i != NULL; i = i->next) PushBack(&letters_dup, i->freq, i->let);

	root = Build(letters);
	DisplayTurn(root, 0);
	char *str = new char[MAX_CODE];
	TraverseTREE(root, 0, &Codes, str);
	for (Letter *i = letters_dup; i != NULL; i = i->next) if (i->freq != 0) printf("'%c' : %lf : %s\n", i->let, i->freq, FindCode(i->let, Codes));
	PrintCodesToFile(inFile, outFile, letters_dup, Codes);

	fclose(inFile);
	fclose(outFile);
	return 1;
}
#pragma endregion

#pragma region Decode
Letter *Readfreq(FILE *inF)
{
	Letter *letters = NULL;
	int count = 0;

	fseek(inF, 0, SEEK_SET);
	fscanf(inF, "%i\n", &count);

	for (int i = 0; i < count; i++)
	{
		char let;
		double freq;
		fscanf(inF, "'%c' %lf\n", &let, &freq);
		PushBack(&letters, freq, let);
	}

	return letters;
}

void Decode(TREE *root, FILE *iF, FILE *oF)
{
	char c;
	TREE *T = root;

	while ((c = getc(iF)) != EOF)
	{
		if (c == '0' && T->left != NULL)
			T = T->left;
		else if (c == '1' && T->right != NULL)
			T = T->right;

		if (T->left == NULL && T->right == NULL)
		{
			fprintf(oF, "%c", T->let);
			T = root;
		}
	}
}

int DecodeFunc(char *inFilename, char *outFilename)
{
	TREE *root = NULL;
	Letter *letters = NULL;
	FILE *inFile = fopen(inFilename, "rt");
	FILE *outFile = fopen(outFilename, "wt");

	if (inFile == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", inFilename);
		return 0;
	}
	if (outFile == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", outFilename);
		return 0;
	}

	letters = Readfreq(inFile);

	root = Build(letters);
	DisplayTurn(root, 0);
	Decode(root, inFile, outFile);

	fclose(inFile);
	fclose(outFile);
	return 1;
}
#pragma endregion

int main(void)
{
	bool exit = false;
	char key;
	char *fromwhere = new char[15];
	char *towhere = new char[15];

	while (!exit)
	{
		printf("\n\n");
		printf("esc | Exit\n");
		printf("1   | Encode Func\n");
		printf("2   | Start File\n");
		printf("3   | Decode Func\n");
		printf("4   | Check Func\n");

		key = _getch();
		switch (key)
		{
		case 27:
			printf("Exit.\n");
			exit = true;
			break;

		case '1':
			strcpy(fromwhere, "1.txt");
			strcpy(towhere, "outputfile.huf");
			if (EncodeFunc(fromwhere, towhere) == 1) printf("File %s sucsessfuly encdoded to %s.\n", fromwhere, towhere);
			else printf("Something went wrong.\n");
			break;

		case '2':
		{
			FILE *inFile = fopen("1.txt", "wt");
			fprintf(inFile, "mama papa!");
			fclose(inFile);
			printf("File sucsessfuly had been created.\n");
		}
		break;

		case '3':
			strcpy(fromwhere, "outputfile.huf");
			strcpy(towhere, "2.txt");
			if (DecodeFunc(fromwhere, towhere) == 1) printf("File %s sucsessfuly decoded to %s.\n", fromwhere, towhere);
			else printf("Something went wrong.\n");
			break;

		case '4':
		{
			strcpy(fromwhere, "1.txt");
			strcpy(towhere, "2.txt");

			int from = CheckSum(fromwhere), to = CheckSum(towhere);
			printf("Amount of every symbol code:         ");
			if (from == to) printf("Every thing is grate(%i==%i)!!!\n", from, to);
			else printf("Something went wrong(%i!=%i).\n", from, to);

			printf("Character-by-character comparison:   ");
			if (Check(fromwhere, towhere) == 1) printf("Every thing is grate!!!\n");
			else printf("Something went wrong.\n");

			from = Check3(fromwhere), to = Check3(towhere);
			printf("Comparing the number of characters:  ");
			if (from == to) printf("Every thing is grate(%i==%i)!!!\n", from, to);
			else printf("Something went wrong.\n");

			break;
		}

		default:
			printf("Wrong symbol\n");
			break;
		}
	}
	return 0;
}