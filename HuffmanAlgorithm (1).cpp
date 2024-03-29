#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <math.h>
#include <windows.h>
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
};

struct TREE
{
	int freq;
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
		fprintf(stderr, "�� ���������� ������� %s\n", Filename);
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
		fprintf(stderr, "�� ���������� ������� %s\n", Filename);
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
		fprintf(stderr, "�� ���������� ������� %s\n", inFilename);
		return 0;
	}
	if (outFile == NULL)
	{
		fprintf(stderr, "�� ���������� ������� %s\n", outFilename);
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

void BubbleSort(Letter *letters)
{
	for (Letter *i = letters; i != NULL; i = i->next)
	{
		for (Letter *j = i; j != NULL; j = j->next)
		{
			if (j->freq < i->freq)
			{
				Letter tmp = *i;
				i->freq = j->freq;
				i->let = j->let;
				j->freq = tmp.freq;
				j->let = tmp.let;
			}
		}
	}
}

void PushFront(Letter **L, int freq, char let)
{
	Letter *newElement = new Letter;
	if (newElement == NULL)
	{
		fprintf(stderr, "���������� �������� %u ���� ��� ���������\n", sizeof(Letter));
		return;
	}

	newElement->let = let;
	newElement->freq = freq;
	newElement->next = (*L);
	*L = newElement;
}

Letter *FindLetter(Letter *letters, char c)
{
	for (Letter *i = letters; i != NULL; i = i->next)
		if (c == i->let)
			return i;
	return NULL;
}

char *FindCode(char c, CODE *Codes, int codes_size)
{
	for (int i = 0; i < codes_size; i++)
		if (c == Codes[i].let) return Codes[i].code;
	return Codes[0].code;
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
				int ffreq = 0, sfreq = 0;
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
		printf("%3i (%c)\n", T->freq, T->let);

		if (T->left != NULL)
			DisplayTurn(T->left, dep + 1);
	}
}
#pragma endregion

#pragma region Encode

void PrintCodesToFile(FILE *inF, FILE *outF, Letter *a, CODE *Codes, int codes_size)
{
	int c = 0;
	for (int i = 0; i < codes_size; i++) c++;
	fprintf(outF, "%i\n", c);

	for (Letter *i = a; i != NULL; i = i->next) if (i->freq != 0) fprintf(outF, "'%c' %s\n", i->let, FindCode(i->let, Codes, codes_size));

	fseek(inF, 0, SEEK_SET);
	while ((c = getc(inF)) != EOF)	fprintf(outF, "%s", FindCode(c, Codes, codes_size));
	fseek(inF, 0, SEEK_SET);
}

Letter *CountFreq(FILE *F)
{
	Letter *letters = NULL;
	int c = 0;

	while ((c = getc(F)) != EOF) {
		Letter *tmp = FindLetter(letters, c);
		if (tmp != NULL) tmp->freq++;
		else PushFront(&letters, 1, c);
	}

	fseek(F, 0, SEEK_SET);
	return letters;
}

void TraverseTREE(TREE *T, int deph, CODE *Codes, char *str, int *num, int codes_size)
{
	if (T != NULL)
	{
		if (!T->left || !T->right)
		{
			char *tmp_code = new char[deph + 1];
			for (int i = 0; i < deph + 1; i++) tmp_code[i] = str[i];
			tmp_code[deph] = 0;

			if ((*num) < codes_size)
			{
				Codes[(*num)].let = T->let;
				Codes[(*num)].code = _strdup(tmp_code);
			}
			else printf("������ � �������� �����.\n");

			*num = (*num)++;
		}
		else
		{
			str[deph] = '1';
			if (T->right != NULL)
				TraverseTREE(T->right, deph + 1, Codes, str, num, codes_size);
			str[deph] = '0';
			if (T->left != NULL)
				TraverseTREE(T->left, deph + 1, Codes, str, num, codes_size);

		}
	}
}

int EncodeFunc(char *inFilename, char *outFilename) {
	Letter *letters = NULL, *letters_dup = NULL;
	CODE *Codes;
	TREE *root = NULL;
	int codes_size = 0, num = 0;
	char *str = new char[MAX_CODE];

	FILE *inFile = fopen(inFilename, "rt");
	FILE *outFile = fopen(outFilename, "wt");

	if (inFile == NULL)
	{
		fprintf(stderr, "�� ���������� ������� %s\n", inFilename);
		return 0;
	}
	if (outFile == NULL)
	{
		fprintf(stderr, "�� ���������� ������� %s\n", outFilename);
		return 0;
	}

	letters = CountFreq(inFile);
	BubbleSort(letters);
	for (Letter *i = letters; i != NULL; i = i->next)
	{
		PushFront(&letters_dup, i->freq, i->let);
		codes_size++;
	}
	Codes = new CODE[codes_size];

	root = Build(letters);
	DisplayTurn(root, 0);
	TraverseTREE(root, 0, Codes, str, &num, codes_size);
	for (Letter *i = letters_dup; i != NULL; i = i->next) if (i->freq != 0) printf("'%c' : %i : %s\n", i->let, i->freq, FindCode(i->let, Codes, codes_size));
	PrintCodesToFile(inFile, outFile, letters_dup, Codes, codes_size);

	fclose(inFile);
	fclose(outFile);
	return 1;
}
#pragma endregion

#pragma region Decode

CODE *ReadCodes(FILE *inF)
{
	CODE *Codes;
	int count = 0;

	fseek(inF, 0, SEEK_SET);
	fscanf(inF, "%i\n", &count);

	Codes = new CODE[count];
	
	for (int i = 0; i < count; i++)
	{
		Codes[i].code = new char[MAX_CODE];
		fscanf(inF, "'%c' %s\n", &Codes[i].let, Codes[i].code);
	}
	
	return Codes;
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

TREE *BuildByCodes(TREE *T, CODE x, int dep)
{
	if (T == NULL)
	{
		T = new TREE();
		T->freq = 0;
		T->let = '-';
		T->right = T->left = NULL;
	}

	char c = x.code[dep];
	if (c == '0') T->left = BuildByCodes(T->left, x, dep + 1);
	else if (c == '1') T->right = BuildByCodes(T->right, x, dep + 1);
	else if (c == 0) T->let = x.let;

	return T;
}

int DecodeFunc(char *inFilename, char *outFilename)
{
	TREE *root = NULL;
	Letter *letters = NULL;
	CODE *Codes;
	FILE *inFile = fopen(inFilename, "rt");
	FILE *outFile = fopen(outFilename, "wt");
	int codes_size = 0;

	if (inFile == NULL)
	{
		fprintf(stderr, "�� ���������� ������� %s\n", inFilename);
		return 0;
	}
	if (outFile == NULL)
	{
		fprintf(stderr, "�� ���������� ������� %s\n", outFilename);
		return 0;
	}

	fseek(inFile, 0, SEEK_SET);
	fscanf(inFile, "%i\n", &codes_size);
	Codes = ReadCodes(inFile);

	for (int i = 0; i < codes_size; i++)
		printf("'%c' %s.\n", Codes[i].let, Codes[i].code);

	for (int i = 0; i < codes_size; i++)
		root = BuildByCodes(root, Codes[i], 0);

	DisplayTurn(root, 0);
	Decode(root, inFile, outFile);

	fclose(inFile);
	fclose(outFile);
	return 1;
}
#pragma endregion

int main(void)
{
	setlocale(LC_ALL, "Russian");
	bool exit = false;
	char key;
	char *fromwhere = new char[15];
	char *towhere = new char[15];

	while (!exit)
	{
		printf("\n\n");
		printf("esc | �����\n");
		printf("1   | ������������ �����\n");
		printf("2   | ������� ����\n");
		printf("3   | ������������� �����\n");
		printf("4   | ��������� ������ ���������\n");

		key = _getch();
		switch (key)
		{
		case 27:
			printf("������.\n");
			exit = true;
			break;

		case '1':
			strcpy(fromwhere, "1.txt");
			strcpy(towhere, "outputfile.huf");
			if (EncodeFunc(fromwhere, towhere) == 1) printf("���� %s ������� ����������� � %s.\n", fromwhere, towhere);
			else printf("���-�� ����� �� ���.\n");
			break;

		case '2':
		{
			FILE *inFile = fopen("1.txt", "wt");
			fprintf(inFile, "mama papa!");
			fclose(inFile);
			printf("���� ��� ������.\n");
		}
		break;

		case '3':
			strcpy(fromwhere, "outputfile.huf");
			strcpy(towhere, "2.txt");
			if (DecodeFunc(fromwhere, towhere) == 1) printf("���� %s ��� ������������ � %s.\n", fromwhere, towhere);
			else printf("���-�� ����� �� ���.\n");
			break;

		case '4':
		{
			strcpy(fromwhere, "1.txt");
			strcpy(towhere, "2.txt");

			int from = CheckSum(fromwhere), to = CheckSum(towhere);
			printf("��������� ������������ ����� �����:  ");
			if (from == to) printf("��� �����(%i==%i)!!!\n", from, to);
			else printf("���-�� ����� �� ���(%i!=%i).\n", from, to);

			printf("������������ ���������:              ");
			if (Check(fromwhere, towhere) == 1) printf("��� �����!!!\n");
			else printf("���-�� ����� �� ���.\n");

			from = Check3(fromwhere), to = Check3(towhere);
			printf("��������� ���������� ��������:       ");
			if (from == to) printf("��� �����(%i==%i)!!!\n", from, to);
			else printf("���-�� ����� �� ���(%i!=%i).\n", from, to);
			break;
		}

		default:
			printf("�� ���������� ��������\n");
			break;
		}
	}
	return 0;
}