/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

void freeObject(Object* obj);
void freeScope(Scope* scope);
void freeObjectList(ObjectNode *objList);
void freeReferenceList(ObjectNode *objList);

SymTab* symtab;
Type* intType;
Type* charType;

/******************* Type utilities ******************************/

Type* makeIntType(void) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_INT;
  return type;
}

Type* makeCharType(void) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_CHAR;
  return type;
}

Type* makeArrayType(int arraySize, Type* elementType) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_ARRAY;
  type->arraySize = arraySize;
  type->elementType = elementType;
  return type;
}

Type* duplicateType(Type* type) {
  Type* t = (Type*) malloc(sizeof(Type));
  t->typeClass = type->typeClass;
  if (t->typeClass == TP_ARRAY) {
    t->arraySize = type->arraySize;
    t->elementType = duplicateType(type->elementType);
  }
  return t;
}

int compareType(Type* type1, Type* type2) {
  if (type1->typeClass == type2->typeClass) {
    if (type1->typeClass == TP_ARRAY) {
      if (type1->arraySize == type2->arraySize) {
        return compareType(type1->elementType, type2->elementType);
      }
      return 0;
    }
    return 1;
  }
  return 0;
}

void freeType(Type* type) {
  switch(type->typeClass){
    case TP_INT:
    case TP_CHAR:
      free(type);
      break;
    case TP_ARRAY:
      freeType(type->elementType);
      free(type);
      break;
  }
}

/******************* Constant utility ******************************/

ConstantValue* makeIntConstant(int i) {
  ConstantValue* val = (ConstantValue*) malloc(sizeof(ConstantValue));
  val->type = TP_INT;
  val->intValue = i;
  return val;
}

ConstantValue* makeCharConstant(char ch) {
  ConstantValue* val = (ConstantValue*) malloc(sizeof(ConstantValue));
  val->type = TP_CHAR;
  val->charValue = ch;
  return val;
}

ConstantValue* duplicateConstantValue(ConstantValue* v) {
  ConstantValue* c = (ConstantValue*) malloc(sizeof(ConstantValue));
  c->type = v->type;
  switch (v->type) {
    case TP_INT: {
      c->intValue = v->intValue;
      break;
    }
    case TP_CHAR: {
      c->charValue = v->charValue;
      break;
    }
    default:
      printf("Invalid constant value\n");
      exit(1);
  }
  return c;
}

/******************* Object utilities ******************************/

Scope* createScope(Object* owner, Scope* outer) {
  Scope* scope = (Scope*) malloc(sizeof(Scope));
  scope->objList = NULL;
  scope->owner = owner;
  scope->outer = outer;
  return scope;
}

Object* createProgramObject(char *programName) {
  Object* program = (Object*) malloc(sizeof(Object));
  strcpy(program->name, programName);
  program->kind = OBJ_PROGRAM;
  program->progAttrs = (ProgramAttributes*) malloc(sizeof(ProgramAttributes));
  program->progAttrs->scope = createScope(program,NULL);
  symtab->program = program;

  return program;
}

Object* createConstantObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  strcpy(obj->name, name);
  obj->kind = OBJ_CONSTANT;
  obj->constAttrs = (ConstantAttributes*) malloc(sizeof(ConstantAttributes));
  obj->constAttrs->value = (ConstantValue*) malloc(sizeof(ConstantValue));
  return obj;
}

Object* createTypeObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  strcpy(obj->name, name);
  obj->kind = OBJ_TYPE;
  obj->typeAttrs = (TypeAttributes*) malloc(sizeof(TypeAttributes));
  obj->typeAttrs->actualType = (Type*) malloc(sizeof(Type));
  return obj;
}

Object* createVariableObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  strcpy(obj->name, name);
  obj->kind = OBJ_VARIABLE;
  obj->varAttrs = (VariableAttributes*) malloc(sizeof(VariableAttributes));
  obj->varAttrs->type = (Type*) malloc(sizeof(Type));
  obj->varAttrs->scope = createScope(obj, symtab->currentScope);
  return obj;
}

Object* createFunctionObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  strcpy(obj->name, name);
  obj->kind = OBJ_FUNCTION;
  obj->funcAttrs = (FunctionAttributes*) malloc(sizeof(FunctionAttributes));
  obj->funcAttrs->paramList = (ObjectNode*) malloc(sizeof(ObjectNode));
  obj->funcAttrs->returnType = (Type*) malloc(sizeof(Type));
  obj->funcAttrs->scope = createScope(obj, symtab->currentScope);
  return obj;
}

Object* createProcedureObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  strcpy(obj->name, name);
  obj->kind = OBJ_PROCEDURE;
  obj->procAttrs = (ProcedureAttributes*) malloc(sizeof(ProcedureAttributes));
  obj->procAttrs->paramList = (ObjectNode*) malloc(sizeof(ObjectNode));
  obj->procAttrs->scope = createScope(obj, symtab->currentScope);
  return obj;
}

Object* createParameterObject(char *name, enum ParamKind kind, Object* owner) {
  Object* obj = (Object*) malloc(sizeof(Object));
  strcpy(obj->name, name);
  obj->kind = OBJ_PARAMETER;
  obj->paramAttrs = (ParameterAttributes*) malloc(sizeof(ParameterAttributes));

  obj->paramAttrs->type = (Type*) malloc(sizeof(Type));
  obj->paramAttrs->kind = kind;
  obj->paramAttrs->function = (Object*) malloc(sizeof(Object));
  obj->paramAttrs->function = owner;
  return obj;
}

void freeObject(Object* obj) {
  switch(obj->kind) {
    case OBJ_TYPE:{
      freeType(obj->typeAttrs->actualType);
      free(obj->typeAttrs);
      free(obj);
      break;
    }
    case OBJ_CONSTANT: {
      free(obj->constAttrs->value);
      free(obj->constAttrs);
      free(obj);
      break;
    }
    case OBJ_PARAMETER: {
      freeType(obj->paramAttrs->type);
      free(obj->paramAttrs->function);
      free(obj->paramAttrs);
      free(obj);
      break;
    }
    case OBJ_PROCEDURE: {
      freeObjectList(obj->procAttrs->paramList);
      freeScope(obj->procAttrs->scope);
      free(obj->procAttrs);
      break;
    }
    case OBJ_FUNCTION: {
      freeScope(obj->funcAttrs->scope);
      freeType(obj->funcAttrs->returnType);
      freeObjectList(obj->funcAttrs->paramList);
      free(obj->funcAttrs);
      free(obj);
      break;
    }
    case OBJ_VARIABLE: {
      freeType(obj->varAttrs->type);
      freeScope(obj->varAttrs->scope);
      free(obj->varAttrs);
      free(obj);
      break;
    }
    case OBJ_PROGRAM: {
      freeScope(obj->progAttrs->scope);
      free(obj->progAttrs);
      free(obj);
      break;
    }
  }
}

void freeScope(Scope* scope) {
  freeObjectList(scope->objList);
  free(scope);
}

void freeObjectList(ObjectNode *objList) {
  ObjectNode* cur = objList;
  while (cur != NULL) {
    ObjectNode* node = cur;
    cur = cur->next;
    free(node);
  }
}

void freeReferenceList(ObjectNode *objList) {
  ObjectNode* cur = objList;
  while (cur != NULL) {
      ObjectNode* node = cur;
      cur = cur->next;
      free(node);
  }
}

void addObject(ObjectNode **objList, Object* obj) {
  ObjectNode* node = (ObjectNode*) malloc(sizeof(ObjectNode));
  node->object = obj;
  node->next = NULL;
  if ((*objList) == NULL)
    *objList = node;
  else {
    ObjectNode *n = *objList;
    while (n->next != NULL)
      n = n->next;
    n->next = node;
  }
}

Object* findObject(ObjectNode *objList, char *name) {
  ObjectNode* cur = objList;
  while (cur != NULL) {
    if (strcmp(cur->object->name, name) == 0) {
      return cur->object;
    }
    cur = cur->next;
  }
  return NULL;
}

/******************* others ******************************/

void initSymTab(void) {
  Object* obj;
  Object* param;

  symtab = (SymTab*) malloc(sizeof(SymTab));
  symtab->globalObjectList = NULL;

  obj = createFunctionObject("READC");
  obj->funcAttrs->returnType = makeCharType();
  addObject(&(symtab->globalObjectList), obj);

  obj = createFunctionObject("READI");
  obj->funcAttrs->returnType = makeIntType();
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITEI");
  param = createParameterObject("i", PARAM_VALUE, obj);
  param->paramAttrs->type = makeIntType();
  addObject(&(obj->procAttrs->paramList),param);
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITEC");
  param = createParameterObject("ch", PARAM_VALUE, obj);
  param->paramAttrs->type = makeCharType();
  addObject(&(obj->procAttrs->paramList),param);
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITELN");
  addObject(&(symtab->globalObjectList), obj);

  intType = makeIntType();
  charType = makeCharType();
}

void cleanSymTab(void) {
  freeObject(symtab->program);
  freeObjectList(symtab->globalObjectList);
  free(symtab);
  freeType(intType);
  freeType(charType);
}

void enterBlock(Scope* scope) {
  symtab->currentScope = scope;
}

void exitBlock(void) {
  symtab->currentScope = symtab->currentScope->outer;
}

void declareObject(Object* obj) {
  if (obj->kind == OBJ_PARAMETER) {
    Object* owner = symtab->currentScope->owner;
    switch (owner->kind) {
    case OBJ_FUNCTION:
      addObject(&(owner->funcAttrs->paramList), obj);
      break;
    case OBJ_PROCEDURE:
      addObject(&(owner->procAttrs->paramList), obj);
      break;
    default:
      break;
    }
  }

  addObject(&(symtab->currentScope->objList), obj);
}
