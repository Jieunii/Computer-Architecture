#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Inst
{
    char *name;
    char *op;
    char type;
    char *funct;
};

struct Inst inst[22] = {
    {"addiu", "001001", 'I', ""},      // t s 3
    {"addu", "000000", 'R', "100001"}, // d s t
    {"and", "000000", 'R', "100100"},  // d s t
    {"andi", "001100", 'I', ""},       // t s 3
    {"beq", "000100", 'I', ""},        // s t 3
    {"bne", "000101", 'I', ""},        // s t 3
    {"j", "000010", 'J', ""},
    {"jal", "000011", 'J', ""},
    {"jr", "000000", 'J', "001000"}, 
    {"lui", "001111", 'I', ""}, // t 2
    {"lw", "100011", 'I', ""},  // t s 2
    {"nor", "000000", 'R', "100111"},
    {"or", "000000", 'R', "100101"},
    {"ori", "001101", 'I', ""},        // t s 3
    {"sltiu", "001011", 'I', ""},      // t s 3
    {"sltu", "000000", 'R', "101011"}, // d s t
    {"sll", "000000", 'S', "000000"},
    {"srl", "000000", 'S', "000010"},
    {"sw", "101011", 'I', ""},          // t s 2
    {"subu", "000000", 'R', "100011"}}; // d s t

struct Data
{
    int value;
    struct Data *next;
};

struct Text
{
    int idx;
    char *d;
    char *s;
    char *t;
    unsigned int address;
    struct Text *next;
};

struct Sym
{
    char *name;
    int size;
    unsigned int address;
    struct Data *first;
    struct Sym *next;
    struct Data *last;
};

struct Sym *Symbols;
struct Text *Texts;
int datasize, textsize;

/*
 * You may need the following function
 */
char *NumToBits(unsigned int num, int len)
{
    char *bits = (char *)malloc(len + 1);
    int idx = len - 1, i;

    while (num > 0 && idx >= 0)
    {
        if (num % 2 == 1)
        {
            bits[idx--] = '1';
        }
        else
        {
            bits[idx--] = '0';
        }
        num /= 2;
    }

    for (i = idx; i >= 0; i--)
    {
        bits[i] = '0';
    }

    return bits;
}

void InsertSym(struct Sym *temp_sym)
{
    struct Sym *current_sym = Symbols;
    while (current_sym->next != NULL)
    {
        current_sym = current_sym->next;
    }
    current_sym->next = temp_sym;
    temp_sym = NULL;
    free(temp_sym);
}

void InsertText(struct Text *temp_text)
{
    struct Text *current_text = Texts;
    while (current_text->next != NULL)
    {
        current_text = current_text->next;
    }
    current_text->next = temp_text;
}

/*
 * read the assmebly code
 */
void read_asm()
{
    int tmp, i;
    unsigned int address;
    char temp[0x1000] = {0};
    struct Sym *temp_sym = NULL;
    struct Data *temp_data = NULL;
    struct Text *temp_text = NULL;

    Symbols->next = NULL;

    //Read .data region
    address = 0x10000000;
    for (i = 0; scanf("%s", temp) == 1;)
    {
        if (strcmp(temp, ".text") == 0)
        {
            break;
        }
        else if (temp[strlen(temp) - 1] == ':')
        {
            if (temp_sym != NULL)
            {
                InsertSym(temp_sym); // Sym 연결
            }
            // Sym 생성
            temp_sym = (struct Sym *)malloc(sizeof(struct Sym));
            temp[strlen(temp) - 1] = 0;
            temp_sym->name = (char *)malloc(strlen(temp));
            strcpy(temp_sym->name, temp);
            temp_sym->address = address;
            temp_sym->size = 0;
            temp_sym->next = NULL;
            temp_sym->first = temp_sym->last = NULL;
        }
        else if (strcmp(temp, ".word") == 0)
        {
            // data 생성
            temp_data = (struct Data *)malloc(sizeof(struct Data));
            temp_data->next = NULL;
            if (scanf("%s", temp) == 1) //.word 다음에 있는 값 확인
            {
                temp_data->value = (int)strtol(temp, NULL, 0); //문자열 해당진수로
            }
            // data 연결
            if (temp_sym->first == NULL)
            {
                temp_sym->first = temp_data;
            }
            else
            {
                struct Data *current_data = temp_sym->first;
                while (current_data->next != NULL)
                {
                    current_data = current_data->next;
                }
                current_data->next = temp_data;
            }
            address += 4;
            temp_sym->size++;
            temp_data = NULL;
            free(temp_data);
        }
    }

    datasize = address - 0x10000000;

    //Read .text region
    address = 0x400000;
    for (i = 0; scanf("%s", temp) == 1;)
    {
        if (temp[strlen(temp) - 1] == ':')
        {
            if (temp_sym != NULL)
            {
                InsertSym(temp_sym); // Sym 연결
            }
            // Sym 생성
            temp_sym = (struct Sym *)malloc(sizeof(struct Sym));
            temp[strlen(temp) - 1] = 0;
            temp_sym->name = (char *)malloc(strlen(temp));
            strcpy(temp_sym->name, temp);
            temp_sym->address = address / 4;
            temp_sym->size = 0;
            temp_sym->next = NULL;
            temp_sym->first = temp_sym->last = NULL;
        }
        else
        {  // text 생성
            temp_text = (struct Text *)malloc(sizeof(struct Text));
            temp_text->next = NULL;
            temp_text->s = temp_text->d = temp_text->t = NULL;
            temp_text->address = address;
            if (strcmp(temp, "la") == 0)
            {
                for (int i = 0; i < 2; i++)
                {
                    scanf("%s", temp);
                    if (temp_text->t == NULL)
                    { 
                        temp_text->t = (char *)malloc(strlen(temp) + 5);
                        strcpy(temp_text->t, temp);
                    }
                    else if (temp_text->d == NULL)
                    {
                        temp_text->d = (char *)malloc(strlen(temp) + 16);
                        strcpy(temp_text->d, temp);
                    }
                }
                temp_text->s = (char *)malloc(strlen("00000") + 5);
                strcpy(temp_text->s, "00000");
                int is_only_lui = 1;
                int ori_address;
                for (struct Sym *sym = Symbols->next; sym != NULL; sym = sym->next)
                {
                    if (strcmp(sym->name, temp_text->d) == 0)
                    { // 하위 16비트 조사
                        ori_address = sym->address;
                        char check[33];
                        strcpy(check, NumToBits(sym->address, 32));
                        for (int i = 0; i < 32; ++i)
                        {
                            if (i < 16) // 상위 16비트 저장
                            {
                                temp_text->d[i] = check[i];
                            }
                            else
                            {
                                if (check[i] != '0')
                                {
                                    is_only_lui = 0;
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
                temp_text->d[16] = 0; // NUL 문자
                int dec = strtol(temp_text->d, NULL, 2); // 상위 16비트 10진수로 변경
                sprintf(temp, "%d", dec);
                strcpy(temp_text->d, temp); // 10진수 문자열 저장
                temp_text->idx = 9; // lui
                temp_text->address = address;
                InsertText(temp_text);
                address += 4;
                if (!is_only_lui)
                { // lui, ori 둘 다
                    struct Text *ori_text = (struct Text *)malloc(sizeof(struct Text));
                    ori_text->address = address;
                    ori_text->next = NULL;
                    ori_text->s = (char *)malloc(strlen(temp_text->t) + 5);
                    strcpy(ori_text->s, temp_text->t);
                    ori_text->t = (char *)malloc(strlen(temp_text->t) + 5);
                    strcpy(ori_text->t, temp_text->t);
                    ori_text->d = (char *)malloc(17);
                    sprintf(temp, "%d", ori_address-0x10000000);
                    strcpy(ori_text->d, temp);
                    ori_text->idx = 13; // ori
                    InsertText(ori_text);
                    ori_text = NULL;
                    free(ori_text);
                    address += 4;
                }
            }
            else
            {
                for (int i = 0; i < 21; ++i)
                {
                    if (strcmp(temp, inst[i].name) == 0)
                    {
                        temp_text->idx = i;
                        break;
                    }
                }
                if (inst[temp_text->idx].type == 'J')
                {   // s
                    scanf("%s", temp);
                    if (temp_text->s == NULL)
                    {
                        temp_text->s = (char *)malloc(strlen(temp) + 26);
                        strcpy(temp_text->s, temp);
                    }
                    InsertText(temp_text);
                }
                else if (inst[temp_text->idx].type == 'R' || inst[temp_text->idx].type == 'S')
                {   // d s t
                    for (int i = 0; i < 3; ++i)
                    {
                        scanf("%s", temp);
                        if (temp_text->d == NULL)
                        {
                            temp_text->d = (char *)malloc(strlen(temp) + 16);
                            strcpy(temp_text->d, temp);
                        }
                        else if (temp_text->s == NULL)
                        {
                            temp_text->s = (char *)malloc(strlen(temp) + 5);
                            strcpy(temp_text->s, temp);
                        }
                        else if (temp_text->t == NULL)
                        {
                            temp_text->t = (char *)malloc(strlen(temp) + 5);
                            strcpy(temp_text->t, temp);
                        }
                    }
                    InsertText(temp_text);
                }
                else if (inst[temp_text->idx].type == 'I')
                {
                    if (temp_text->idx != 10 && temp_text->idx != 18) // lw, sw가 아닌 경우
                    {    // t s d or t d s
                        for (int i = 0; i < 3; i++)
                        {
                            scanf("%s", temp);
                            if (temp_text->t == NULL)
                            {
                                temp_text->t = (char *)malloc(strlen(temp) + 5);
                                strcpy(temp_text->t, temp);
                            }
                            else if (temp_text->s == NULL && temp_text->idx != 9) // lui가 아닐 때
                            {                                
                                temp_text->s = (char *)malloc(strlen(temp) + 5);
                                strcpy(temp_text->s, temp);                                
                            }
                            else if (temp_text->d == NULL)
                            {
                                temp_text->d = (char *)malloc(strlen(temp) + 16);
                                strcpy(temp_text->d, temp);
                                if(temp_text->idx == 9) { // lui 일 때 
                                    temp_text->s = (char *)malloc(6);
                                    strcpy(temp_text->s, "0");   
                                    break;                             
                                }
                            }
                        }
                    }
                    else // lw, sw 
                    {   // t  s  d
                        for (int i = 0; i < 2; ++i)
                        {
                            scanf("%s", temp);
                            if (temp_text->t == NULL)
                            {
                                temp_text->t = (char *)malloc(strlen(temp) + 5);
                                strcpy(temp_text->t, temp);
                            }
                            else if (temp_text->s == NULL)\
                            {
                                char *ptr = strchr(temp, '$');
                                temp_text->s = (char *)malloc(strlen(ptr) + 5);
                                strcpy(temp_text->s, ptr + 1);
                            }
                        }
                        temp_text->d = (char *)malloc(strlen(temp) + 16);
                        strcpy(temp_text->d, temp);
                    }
                    InsertText(temp_text);
                }
                address += 4;
            }
            temp_text = NULL;
            free(temp_text);
        }
    }
    if (temp_sym != NULL)
    {
        InsertSym(temp_sym); // Sym 연결
    }
    textsize = address - 0x400000;
}

/*
 * convert the assembly code to num
 */
void subst_asm_to_num()
{
    struct Text *text;
    struct Sym *sym;
    char temp[0x1000] = {0};
    for (text = Texts->next; text != NULL; text = text->next)
    {
        if (text->s[0] == '$')
                strcpy(text->s, (text->s) + 1);
        if(inst[text->idx].type != 'J') {
            if (text->t[0] == '$')
                strcpy(text->t, (text->t) + 1);
            if (text->d[0] == '$')
                strcpy(text->d, (text->d) + 1);
        }
        
        for (sym = Symbols->next; sym != NULL; sym = sym->next)
        {
            if(inst[text->idx].type == 'J') { 
                if (strcmp(text->s, sym->name) == 0)
                { 
                    strcpy(text->s, NumToBits(sym->address, 26));
                    break;
                }
            }
            else {
                if (strcmp(text->d, sym->name) == 0) // beq, bne
                {
                    strcpy(text->s, NumToBits((int)strtol(text->s, NULL, 0), 5));
                    strcpy(text->t, NumToBits((int)strtol(text->t, NULL, 0), 5));
                    strcpy(text->d, NumToBits(sym->address - text->address / 4 - 1, 16));
                    break;
                }
            }
        }
        if (sym == NULL) // symbol이 없을 때
        {
            strcpy(text->s, NumToBits((int)strtol(text->s, NULL, 0), 5));
            if (inst[text->idx].type != 'J')
            {
                strcpy(text->t, NumToBits((int)strtol(text->t, NULL, 0), 5));
                if (inst[text->idx].type == 'I')
                {
                    strcpy(text->d, NumToBits((int)strtol(text->d, NULL, 0), 16));
                }
                else
                {
                    strcpy(text->d, NumToBits((int)strtol(text->d, NULL, 0), 5));
                }
            }
        }
    }
}

/*
 * print the results of assemble
 */
void print_bits()
{
    struct Text *text;
    struct Sym *sym;
    struct Data *data;
    // print the headshifter
    printf("%s", NumToBits(textsize, 32));
    printf("%s", NumToBits(datasize, 32));

    // print the body
    for (text = Texts->next; text != NULL; text = text->next)
    {
        printf("%s", inst[text->idx].op);

        if (inst[text->idx].type == 'R')
        {
            printf("%s%s%s", text->s, text->t, text->d);
            printf("00000%s", inst[text->idx].funct);
        }
        else if (inst[text->idx].type == 'I')
        {
            if(text->idx == 4 || text->idx == 5) { // bne, beq            
                printf("%s%s%s", text->t, text->s, text->d);
            } else {
                printf("%s%s%s", text->s, text->t, text->d);
            }
        }
        else if (inst[text->idx].type == 'S')
        {
            printf("00000%s%s%s", text->s, text->d, text->t);
            printf("%s", inst[text->idx].funct);
        }
        else
        {
            printf("%s", text->s);
            if(strlen(text->s) == 5) {
                printf("000000000000000%s", inst[text->idx].funct);
            }
        }
    }
    // data section은 마지막에 출력
    for (sym = Symbols->next; sym != NULL; sym = sym->next)
    {
        for (data = sym->first; data != sym->last; data = data->next)
        {
            if (data)
            {
                printf("%s", NumToBits(data->value, 32));
            }
        }
    }
    printf("\n");
}

/*
 * main function
 */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./assembler <assembly file>\n");
        exit(0);
    }
    else
    {
        // reading the input file
        char *filename = (char *)malloc(strlen(argv[1]) + 3);
        strncpy(filename, argv[1], strlen(argv[1]));

        if (freopen(filename, "r", stdin) == 0)
        {
            printf("File open Error\n");
            exit(1);
        }

        Symbols = (struct Sym *)malloc(sizeof(struct Sym));
        Texts = (struct Text *)malloc(sizeof(struct Text));
        // creating the output file (*.o)
        filename[strlen(filename) - 1] = 'o';
        freopen(filename, "w", stdout);

        // Let's complete the below functions!
        read_asm();
        subst_asm_to_num();
        print_bits();

        // freeing the memory
        free(filename);
        free(Symbols);
        free(Texts);
    }
    return 0;
}