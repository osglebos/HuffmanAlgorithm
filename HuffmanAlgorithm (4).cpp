#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <math.h>
#include <windows.h>
#include <list>
#define MAX_CODE 44
#define MAX_POW 100000

using namespace std;

struct Letter
{
	double freq;
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

	TREE(double Freq, char Let, TREE *Left, TREE *Right) : freq(Freq), let(Let), left(Left), right(Right) { }
	TREE(TREE *T) : freq(T->freq), let(T->let), left(T->left), right(T->right) { }
	TREE() : freq(0), let('-'), left(NULL), right(NULL) { }
};

#pragma region Check

int CheckSum(char *Filename)
{
	int sum = 0;
	int c = 0;
	FILE *File = fopen(Filename, "rt");

	if (File == NULL)
	{
		fprintf(stderr, "Не получилось открыть %s\n", Filename);
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
		fprintf(stderr, "Не получилось открыть %s\n", Filename);
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
		fprintf(stderr, "Не получилось открыть %s\n", inFilename);
		return 0;
	}
	if (outFile == NULL)
	{
		fprintf(stderr, "Не получилось открыть %s\n", outFilename);
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

void AddLetter(Letter **L, double freq, char let)
{
	Letter *newElement = new Letter;
	if (newElement == NULL)
	{
		fprintf(stderr, "Невозможно выделить %u байт для выражения\n", sizeof(Letter));
		return;
	}
	while (*L != NULL)
		if ((*L)->let > let)
			L = &((*L)->next);
		else
			break;

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

char *FindCode(char c, CODE *Codes)
{
	for (CODE *i = Codes; i != NULL; i = i->next)
		if (c == i->let) return i->code;
	return Codes->code;
}

void AddCode(CODE **L, char let, char *str)
{
	CODE *newElement = new CODE;
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

TREE *BuildHuf(Letter *a)
{
	list<TREE*> letters;

	for (Letter *i = a; i != NULL; i = i->next)
		letters.push_back(new TREE(i->freq, i->let, NULL, NULL));
			
	while (letters.size() != 1)
	{
		TREE *first = *(letters.begin());
		TREE *second = *(++letters.begin());
		
		if (letters.size() != 2)
		{
			list<TREE *>::iterator min;

			min = letters.begin();
			for (list<TREE *>::iterator i = letters.begin(); i != letters.end(); ++i)
				if ((*min)->freq > (*i)->freq)
					min = i;
			first = new TREE(*min);
			letters.erase(min);

			min = letters.begin();
			for (list<TREE *>::iterator i = letters.begin(); i != letters.end(); ++i)
				if ((*min)->freq > (*i)->freq)
					min = i;
			second = new TREE(*min);
			letters.erase(min);
		}
		else return new TREE((second->freq + first->freq), '-', first, second);
		
		letters.push_back(new TREE((second->freq + first->freq), '-', first, second));
	}
	return new TREE((*(letters.begin()))->freq, (*(letters.begin()))->let, NULL, NULL);
}

#pragma endregion

#pragma region Encode

void PrintCodesToFile(FILE *inF, FILE *outF, Letter *a, CODE *Codes)
{
	int c = 0;
	for (CODE *i = Codes; i != NULL; i = i->next) c++;
	fprintf(outF, "%i\n", c);

	for (Letter *i = a; i != NULL; i = i->next) if (i->freq != 0) fprintf(outF, "'%c' %s\n", i->let, FindCode(i->let, Codes));

	fseek(inF, 0, SEEK_SET);
	while ((c = getc(inF)) != EOF)	fprintf(outF, "%s", FindCode(c, Codes));
	fseek(inF, 0, SEEK_SET);
}

Letter *CountFreq(FILE *F)
{
	Letter *letters = NULL;
	int c = 0;
	int sum = 0;

	while ((c = getc(F)) != EOF) {
		Letter *tmp = FindLetter(letters, c);
		sum++;
		if (tmp != NULL) tmp->freq++;
		else AddLetter(&letters, 1, c);
	}

	for (Letter *i = letters; i != NULL; i = i->next)
		i->freq = floor(i->freq * MAX_POW / sum) / MAX_POW;

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
	Letter *letters = NULL, *letters_dup = NULL;
	CODE *Codes = NULL;
	TREE *root = NULL;
	char *str = new char[MAX_CODE];

	FILE *inFile = fopen(inFilename, "rt");
	FILE *outFile = fopen(outFilename, "wt");

	if (inFile == NULL)
	{
		fprintf(stderr, "Не получилось открыть %s\n", inFilename);
		return 0;
	}
	if (outFile == NULL)
	{
		fprintf(stderr, "Не получилось открыть %s\n", outFilename);
		return 0;
	}

	letters = CountFreq(inFile);
	for (Letter *i = letters; i != NULL; i = i->next)
		AddLetter(&letters_dup, i->freq, i->let);
	
	root = BuildHuf(letters);
	DisplayTurn(root, 0);
	TraverseTREE(root, 0, &Codes, str);
	for (Letter *i = letters_dup; i != NULL; i = i->next) if (i->freq != 0) printf("'%c' : %1.5lf : %s\n", i->let, i->freq, FindCode(i->let, Codes));
	PrintCodesToFile(inFile, outFile, letters_dup, Codes);

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
		fprintf(stderr, "Не получилось открыть %s\n", inFilename);
		return 0;
	}
	if (outFile == NULL)
	{
		fprintf(stderr, "Не получилось открыть %s\n", outFilename);
		return 0;
	}

	fseek(inFile, 0, SEEK_SET);
	fscanf(inFile, "%i\n", &codes_size);
	Codes = ReadCodes(inFile);

	for (int i = 0; i < codes_size; i++)
		root = BuildByCodes(root, Codes[i], 0);
	
	DisplayTurn(root, 0);
	for (int i = 0; i < codes_size; i++)
		printf("'%c' %s.\n", Codes[i].let, Codes[i].code);
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
		printf("esc | Выход\n");
		printf("1   | Закодировать текст\n");
		printf("2   | Создать файл\n");
		printf("3   | Раскодировать текст\n");
		printf("4   | Проверить работу программы\n");

		key = _getch();
		switch (key)
		{
		case 27:
			printf("Выхожу.\n");
			exit = true;
			break;

		case '1':
			strcpy(fromwhere, "1.txt");
			strcpy(towhere, "outputfile.huf");
			if (EncodeFunc(fromwhere, towhere) == 1) printf("Фаил %s успешно закодирован в %s.\n", fromwhere, towhere);
			else printf("Что-то пошло не так.\n");
			break;

		case '2':
		{
			FILE *inFile = fopen("1.txt", "wt");
			fprintf(inFile, "mama papa!");
			fclose(inFile);
			printf("Файл был создан.\n");
		}
		break;

		case '3':
			strcpy(fromwhere, "outputfile.huf");
			strcpy(towhere, "2.txt");
			if (DecodeFunc(fromwhere, towhere) == 1) printf("Файл %s был раскодирован в %s.\n", fromwhere, towhere);
			else printf("Что-то пошло не так.\n");
			break;

		case '4':
		{
			strcpy(fromwhere, "1.txt");
			strcpy(towhere, "2.txt");

			int from = CheckSum(fromwhere), to = CheckSum(towhere);
			printf("Сравнение посимвольной суммы кодов:  ");
			if (from == to) printf("Все супер(%i==%i)!!!\n", from, to);
			else printf("Что-то пошло не так(%i!=%i).\n", from, to);

			printf("Посимвольное сравнение:              ");
			if (Check(fromwhere, towhere) == 1) printf("Все супер!!!\n");
			else printf("Что-то пошло не так.\n");

			from = Check3(fromwhere), to = Check3(towhere);
			printf("Сравнение количества символов:       ");
			if (from == to) printf("Все супер(%i==%i)!!!\n", from, to);
			else printf("Что-то пошло не так(%i!=%i).\n", from, to);
			break;
		}

		default:
			printf("Не правильная операция\n");
			break;
		}
	}
	return 0;
}