#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/time.h>
#include<sys/types.h>

#define TEXT_SIZE 256
#define FUNCTION_MAX_COUNT 256
#define TYPE_COUNT 2

#define STRING_TYPE 1
#define INT_TYPE 2

typedef struct TypeStructure Type;
typedef struct ObjectStructure Object;
typedef struct FunctionStructure Function;

struct TypeStructure {

    int typeId;

    char name[TEXT_SIZE];
    Function *functions[FUNCTION_MAX_COUNT];
};


struct ObjectStructure {

    int typeId;

    char sValue[TEXT_SIZE];
    int iValue;
};

struct FunctionStructure {
    char name[TEXT_SIZE];

    Object *(*impl)(Object *);
};

Type types[TYPE_COUNT];

void reportOutOfMemoryAndExit() {
    fprintf(stderr, "Out of memory, bye!");
    exit(1);
};

void reportFunctionNotFound(char *fn) {
    fprintf(stderr, "Could not find function %s, bye!", fn);
    exit(1);
};

Object *allocateNewInt(int v) {

    Object *newInt = (Object *) malloc(sizeof(Object));
    if (newInt == NULL) {
        reportOutOfMemoryAndExit();
    }

    newInt->iValue = v;
    newInt->typeId = INT_TYPE;

    return newInt;
};

Object *allocateNewString(char *text) {

    Object *newString = (Object *) malloc(sizeof(Object));
    if (newString == NULL) {
        reportOutOfMemoryAndExit();
    }

    strcpy(newString->sValue, text);
    newString->typeId = STRING_TYPE;

    return newString;
};

Object *printInt(Object *i) {
    //printf("%d",i->iValue);
    return NULL;
}

Object *printString(Object *s) {
    //printf("%s", s->sValue);
    return NULL;
}

Function *createFunction(char *n, Object *(*f)(Object *)) {

    Function *newFunction = (Function *) malloc(sizeof(Function));
    if (newFunction == NULL) {
        reportOutOfMemoryAndExit();
    }

    strcpy(newFunction->name, n);
    newFunction->impl = f;

    return newFunction;
}

typedef Object *(*o2o)(Object *);

struct cache_element {
    int type_id;
    // This is actually redundand, because normally each place where a function is called would have its own cache
    // so always the same function would be called and there would be no need to check the name!
    char * fun_name;
    o2o impl;
};

typedef struct cache_element cache_element;


int cache_size = 3;
cache_element * lookup_cache [] = {NULL, NULL, NULL};

o2o lookupCalee(Object *o, char *fn) {
    //lookup in the cache
    for (int i = 0; i < cache_size; i++) {
        if (lookup_cache[i] != NULL && lookup_cache[i]->type_id == o->typeId && !strcmp(lookup_cache[i]->fun_name, fn)) {
            return lookup_cache[i]->impl;
        }
    }
    for (int i = 0; i < TYPE_COUNT; i++) {
        if (types[i].typeId == o->typeId) {
            //printf("type found at position %d: %s\n", i, types[i].name);
            for (int j = 0; j < FUNCTION_MAX_COUNT; j++) {
                if (types[i].functions[j] != NULL && !strcmp(fn, types[i].functions[j]->name)) {
                    //   printf("function found at position %d\n", j);
                    o2o fun = types[i].functions[j]->impl;
                    for (int i = 0; i < cache_size; i++) {
                        if (lookup_cache[i] == NULL || i == cache_size -1) {
                            lookup_cache[i] = (cache_element*)malloc(sizeof(cache_element));
                            lookup_cache[i]->fun_name = fn;
                            lookup_cache[i]->type_id = o->typeId;
                            lookup_cache[i]->impl = fun;
                        }
                    }
                    return fun;
                }
            }
        }
    }
    return NULL;
}

// "call site"
Object *invokeFunction(Object *o, char *fn) {
    Object *(*calee)(Object *) = lookupCalee(o, fn);
    if (calee != NULL) {
        calee(o);
    } else {
        reportFunctionNotFound(fn);
    }
}

int main(int argc, char **argv) {

    // init functions:
    Function *printStringFunction = createFunction("print", printString);
    Function *printIntFunction = createFunction("print", printInt);

    // init types
    Type stringType, intType;
    stringType.typeId = STRING_TYPE;
    strcpy(stringType.name, "String");
    memset(stringType.functions, 0, sizeof(stringType.functions));
    stringType.functions[100] = printStringFunction;

    intType.typeId = INT_TYPE;
    strcpy(intType.name, "Int");
    memset(intType.functions, 0, sizeof(intType.functions));
    intType.functions[124] = printIntFunction;

    types[0] = intType;
    types[1] = stringType;

    // START

    struct timeval tval_before, tval_after, tval_result;


    // var s = "FIT";
    Object *s = allocateNewString("FIT");
    // var i = 42;
    Object *i = allocateNewInt(42);
    while (true) {
        gettimeofday(&tval_before, NULL);
        Object *o = s;
        Object *next = i;
        for (int i = 0; i < 2000000; i++) {
            if (!(rand() % 100)) { // o <-> next
                Object *x = o;
                o = next;
                next = x;
            }
            // o.print();
            invokeFunction(o, "print");
        }

        // END

        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);
        printf("\n\nTime elapsed: %ld.%06ld\n", (long int) tval_result.tv_sec, (long int) tval_result.tv_usec);
    }
    return 0;
};
