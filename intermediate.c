
#include "intermediate.h"
#include "assert.h"

static int temporary = 0;
static int label = 0;
static int flag_param = 0;
int test_count=0;
int number_of_quadruples = 0;
FILE *file_quadruples = NULL;
FILE *file_read_quadruples = NULL;

int counter = 0;
int flag_later = 0;
int indx = 0;
int index_parameters = 0;

int check_if_int(TipoLista *table, char name[]){
  int hash = string2int(name) % 211;
  for(TipoID *item = table[hash].start; item != NULL; item = item->prox) {
    if(!strcmp(item->nomeID, name)){
      if (!strcmp(item->tipoID, "func")) {
        if (!strcmp(item->tipoData, "int")) {
          return 1;
        }
      }
    }
  }
  return 0;
}

void add_parameters_to_table(TipoLista *table, list_quadruple *quad_list) {

 
  for (quadruple *p = quad_list->start; p!= NULL; p = p->next){    
    switch (p->op) {
      case PrmVarK:
      case PrmArrK:{
        int hash = string2int(p->address_1.name)%211;
        for (TipoID *q = table[hash].start; q != NULL; q = q->prox) {
          if (!strcmp(p->address_1.name, q->nomeID)&&!strcmp(p->address_2.name, q->escopo)) {
            q->indice_parametro = p->address_3.value;
            break;
          }
        }
        // printf("%s %s %d\n", p->address_1.name, p->address_2.name, p->address_3.value);
        break;
      }
      default:
        break;
    }
  }
}

void add_indexes_to_table(TipoLista *table, list_quadruple *quad_list) {
  quadruple *main_id = NULL;

  for (quadruple *p = quad_list->start; p != NULL; p = p->next){
    if (p->op==LblK) {
        if (p->address_3.kind==String) {
          int hash = string2int(p->address_3.name)%211;
          TipoID *previous_function = NULL;
          for (TipoID *q = table[hash].start; q!= NULL; q = q->prox){
            if (!strcmp(q->tipoID,"func")&&!strcmp(q->nomeID,p->address_3.name)) {
              if(previous_function!=NULL){
                previous_function->intermediate_finish = p->index-1;
              }
              q->intermediate_start = p->index;
              previous_function = q;
              break;
            }
          }
        }
    }
    main_id = p;
  }
  assert(main_id != NULL);
  int hash = string2int("main")%211;
  TipoID *q = table[hash].start;
  assert(q != NULL);
  q->intermediate_finish = main_id->index;
}

void store_quadruple(OpKind o, AddrKind k1, AddrKind k2, AddrKind k3,
    int v1, int v2, int v3,
    char n1[], char n2[], char n3[]){
    // if(k1!=IntConst)
    //   v1 = 0;
    // if(k2!=IntConst)
    //   v2 = 0;
    // if(k3!=IntConst)
    //   v3 = 0;
    if(k1!=String)
      strcpy(n1, "empty");
    if(k2!=String)
      strcpy(n2, "empty");
    if(k3!=String)
      strcpy(n3, "empty");
    number_of_quadruples++;
      fprintf(file_quadruples, "%d %d %d %d %d %d %d %s %s %s ",
      o, k1, k2, k3, v1, v2, v3, n1, n2, n3);
}

void insert_quadruple(list_quadruple *quad_list, quadruple *quad){
   quadruple *p = quad_list->start;
   quadruple *alloc_quad = malloc(sizeof(quadruple));
   memcpy(alloc_quad, quad, sizeof(quadruple));
  //  printf("                               %d\n", quad->op);
   int breaker = 0;
   alloc_quad->index = indx;
   indx++;
   if(p==NULL){
     quad_list->start = alloc_quad;
     quad_list->start->next = NULL;
   }
   else{
     while(p->next!=NULL){
       p = p->next;
     }
      p->next = alloc_quad;
      p->next->next = NULL;
   }
}

void insert_quadruples(list_quadruple *quad_list){
  flag_later = 1;
  char word[1024];
  int item;
  item = 0;
  // quad_list->start = NULL;
  // quad = NULL;
  quadruple *quad = malloc(sizeof(quadruple));


  while (fscanf(file_read_quadruples, "%1023s", word) == 1) {
    // quad->next = NULL;
    // printf("%d %s\n", item, word);
    switch (item) {
      case 0:
        quad->op = (OpKind)atoi(word);
        // printf("%d\n", quad->op);
        item++;
        break;
      case 1:
        quad->address_1.kind = (AddrKind)atoi(word);
        item++;
        break;
      case 2:
        quad->address_2.kind = (AddrKind)atoi(word);
        item++;
        break;
      case 3:
        quad->address_3.kind = (AddrKind)atoi(word);
        item++;
        break;
      case 4:
        quad->address_1.value = atoi(word);
        item++;
        break;
      case 5:
        quad->address_2.value = atoi(word);
        item++;
        break;
      case 6:
        quad->address_3.value = atoi(word);
        item++;
        break;
      case 7:
        strcpy(quad->address_1.name, word);
        item++;
        break;
      case 8:
        strcpy(quad->address_2.name, word);
        item++;
        break;
      case 9:
        strcpy(quad->address_3.name, word);
        // printf("%d\n", item);
        item = 0;
        insert_quadruple(quad_list, quad);
        quadruple *quad = malloc(sizeof(quadruple));
        break;
      default:
        break;
    }
  }
}

void print_quadruple_list(list_quadruple *quad_list){
  for (quadruple *p = quad_list->start; p != NULL; p = p->next) {
    printf("index: %d \toperation: %d \tscope: %s\n \t1: Kind: %d \tName:%s \tValue: %d\n \t2: Kind: %d \tName:%s \tValue: %d\n \t3: Kind: %d \tName:%s \tValue: %d\n",
    p->index, p->op, p->scope,
    p->address_1.kind, p->address_1.name, p->address_1.value,
    p->address_2.kind, p->address_2.name, p->address_2.value,
    p->address_3.kind, p->address_3.name, p->address_3.value);
  }
}

static void generate_relop(list_quadruple *quad_list, TreeNode *tree, TipoLista *table) {
  TreeNode *c0, *c1, *c2;
  c0 = tree->child[0];
  c1 = tree->child[1];
  c2 = tree->child[2];
  int aux;

  quadruple *quad = malloc(sizeof(quadruple));

  switch (tree->attr.oprtr) {
    case EQ:
    if(c0){
      if(c0->kind.stmt == VarK){
        strcpy(quad->address_1.name, c0->attr.name);
        quad->address_1.kind = String;
        strcpy(quad->scope, tree->scope);

        // printf("%s\n", quad->address_1.name);
      }else if (c0->kind.exp == ConstK) {
        quad->address_1.value = c0->attr.value;
        quad->address_1.kind = IntConst;
        // printf("%d\n", quad->address_1.value);
      }else{
        generate_intermediate_code(quad_list, c0, table);
        // printf("%d\n", temporary);
        quad->address_1.value = temporary;
        quad->address_1.kind = Temp;
        // temporary++;
      }
    }

    if(c1){
      if(c1->kind.stmt == VarK){
        strcpy(quad->address_2.name, c1->attr.name);
        quad->address_2.kind = String;
        strcpy(quad->scope, tree->scope);

        // printf("%s\n", quad->address_2.name);
      }else if (c1->kind.exp == ConstK) {
        quad->address_2.value = c1->attr.value;
        quad->address_2.kind = IntConst;
        // printf("%d\n", quad->address_2.value);
      }else{
        generate_intermediate_code(quad_list, c1, table);
        // printf("%d\n", temporary);
        quad->address_2.value = temporary;
        quad->address_2.kind = Temp;
        // temporary++;
      }
    }
    temporary++;
    quad->address_3.value = temporary;
    quad->address_3.kind = Temp;

    quad->op = EqlK;
    switch (quad->address_3.kind) {
      case String: printf("%s ", quad->address_3.name);
        break;
        case IntConst: printf("%d ", quad->address_3.value);
        break;
        case Temp: printf("t%d ", quad->address_3.value);
        default: break;
      }
      printf("= ");

    switch (quad->address_1.kind) {
      case String: printf("%s ", quad->address_1.name);
        break;
      case IntConst: printf("%d ", quad->address_1.value);
        break;
      case Temp: printf("t%d ", quad->address_1.value);
      default: break;
    }

    printf("== ");
    switch (quad->address_2.kind) {
      case String: printf("%s ", quad->address_2.name);
        break;
      case IntConst: printf("%d ", quad->address_2.value);
        break;
      case Temp: printf("t%d ", quad->address_2.value);
      default: break;
    }
    printf("\n");

    insert_quadruple(quad_list, quad);
    store_quadruple(quad->op, quad->address_1.kind, quad->address_2.kind, quad->address_3.kind,
                    quad->address_1.value, quad->address_2.value, quad->address_3.value,
                    quad->address_1.name, quad->address_2.name, quad->address_3.name);
      break;
    case NEQ:
    if(c0){
      if(c0->kind.stmt == VarK){
        strcpy(quad->address_1.name, c0->attr.name);
        quad->address_1.kind = String;
        strcpy(quad->scope, tree->scope);

        // printf("%s\n", quad->address_1.name);
      }else if (c0->kind.exp == ConstK) {
        quad->address_1.value = c0->attr.value;
        quad->address_1.kind = IntConst;
        // printf("%d\n", quad->address_1.value);
      }else{
        generate_intermediate_code(quad_list, c0, table);
        // printf("%d\n", temporary);
        quad->address_1.value = temporary;
        quad->address_1.kind = Temp;
        // temporary++;
      }
    }

    if(c1){
      if(c1->kind.stmt == VarK){
        strcpy(quad->address_2.name, c1->attr.name);
        quad->address_2.kind = String;
        strcpy(quad->scope, tree->scope);

        // printf("%s\n", quad->address_2.name);
      }else if (c1->kind.exp == ConstK) {
        quad->address_2.value = c1->attr.value;
        quad->address_2.kind = IntConst;
        // printf("%d\n", quad->address_2.value);
      }else{
        generate_intermediate_code(quad_list, c1, table);
        // printf("%d\n", temporary);
        quad->address_2.value = temporary;
        quad->address_2.kind = Temp;
        // temporary++;
      }
    }
    temporary++;
    quad->address_3.value = temporary;
    quad->address_3.kind = Temp;

    quad->op = NeqK;
    insert_quadruple(quad_list, quad);
    store_quadruple(quad->op, quad->address_1.kind, quad->address_2.kind, quad->address_3.kind,
                    quad->address_1.value, quad->address_2.value, quad->address_3.value,
                    quad->address_1.name, quad->address_2.name, quad->address_3.name);


                    switch (quad->address_3.kind) {
                      case String: printf("%s ", quad->address_3.name);
                        break;
                        case IntConst: printf("%d ", quad->address_3.value);
                        break;
                        case Temp: printf("t%d ", quad->address_3.value);
                        default: break;
                      }
                      printf("= ");

                    switch (quad->address_1.kind) {
                      case String: printf("%s ", quad->address_1.name);
                        break;
                      case IntConst: printf("%d ", quad->address_1.value);
                        break;
                      case Temp: printf("t%d ", quad->address_1.value);
                      default: break;
                    }

                    printf("!= ");
                    switch (quad->address_2.kind) {
                      case String: printf("%s ", quad->address_2.name);
                        break;
                      case IntConst: printf("%d ", quad->address_2.value);
                        break;
                      case Temp: printf("t%d ", quad->address_2.value);
                      default: break;
                    }
                    printf("\n");
    break;
    case LT:
      if(c0){
        if(c0->kind.stmt == VarK){
          strcpy(quad->address_1.name, c0->attr.name);
          quad->address_1.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_1.name);
        }else if (c0->kind.exp == ConstK) {
          quad->address_1.value = c0->attr.value;
          quad->address_1.kind = IntConst;
          // printf("%d\n", quad->address_1.value);
        }else{
          generate_intermediate_code(quad_list, c0, table);
          // printf("%d\n", temporary);
          quad->address_1.value = temporary;
          quad->address_1.kind = Temp;
          // temporary++;
        }
      }

      if(c1){
        if(c1->kind.stmt == VarK){
          strcpy(quad->address_2.name, c1->attr.name);
          quad->address_2.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_2.name);
        }else if (c1->kind.exp == ConstK) {
          quad->address_2.value = c1->attr.value;
          quad->address_2.kind = IntConst;
          // printf("%d\n", quad->address_2.value);
        }else{
          generate_intermediate_code(quad_list, c1, table);
          // printf("%d\n", temporary);
          quad->address_2.value = temporary;
          quad->address_2.kind = Temp;
          // temporary++;
        }
      }
      temporary++;
      quad->address_3.value = temporary;
      quad->address_3.kind = Temp;

      quad->op = LsrK;
      insert_quadruple(quad_list, quad);
      store_quadruple(quad->op, quad->address_1.kind, quad->address_2.kind, quad->address_3.kind,
                      quad->address_1.value, quad->address_2.value, quad->address_3.value,
                      quad->address_1.name, quad->address_2.name, quad->address_3.name);
                      switch (quad->address_3.kind) {
                        case String: printf("%s ", quad->address_3.name);
                          break;
                          case IntConst: printf("%d ", quad->address_3.value);
                          break;
                          case Temp: printf("t%d ", quad->address_3.value);
                          default: break;
                        }
                        printf("= ");

                      switch (quad->address_1.kind) {
                        case String: printf("%s ", quad->address_1.name);
                          break;
                        case IntConst: printf("%d ", quad->address_1.value);
                          break;
                        case Temp: printf("t%d ", quad->address_1.value);
                        default: break;
                      }

                      printf("< ");
                      switch (quad->address_2.kind) {
                        case String: printf("%s ", quad->address_2.name);
                          break;
                        case IntConst: printf("%d ", quad->address_2.value);
                          break;
                        case Temp: printf("t%d ", quad->address_2.value);
                        default: break;
                      }
                      printf("\n");
      break;
    case LET:
      if(c0){
        if(c0->kind.stmt == VarK){
          strcpy(quad->address_1.name, c0->attr.name);
          quad->address_1.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_1.name);
        }else if (c0->kind.exp == ConstK) {
          quad->address_1.value = c0->attr.value;
          quad->address_1.kind = IntConst;
          // printf("%d\n", quad->address_1.value);
        }else{
          generate_intermediate_code(quad_list, c0, table);
          // printf("%d\n", temporary);
          quad->address_1.value = temporary;
          quad->address_1.kind = Temp;
          // temporary++;
        }
      }

      if(c1){
        if(c1->kind.stmt == VarK){
          strcpy(quad->address_2.name, c1->attr.name);
          quad->address_2.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_2.name);
        }else if (c1->kind.exp == ConstK) {
          quad->address_2.value = c1->attr.value;
          quad->address_2.kind = IntConst;
          // printf("%d\n", quad->address_2.value);
        }else{
          generate_intermediate_code(quad_list, c1, table);
          // printf("%d\n", temporary);
          quad->address_2.value = temporary;
          quad->address_2.kind = Temp;
          // temporary++;
        }
      }
      temporary++;
      quad->address_3.value = temporary;
      quad->address_3.kind = Temp;

      quad->op = LeqK;
      insert_quadruple(quad_list, quad);
      store_quadruple(quad->op, quad->address_1.kind, quad->address_2.kind, quad->address_3.kind,
                      quad->address_1.value, quad->address_2.value, quad->address_3.value,
                      quad->address_1.name, quad->address_2.name, quad->address_3.name);
                      switch (quad->address_3.kind) {
                        case String: printf("%s ", quad->address_3.name);
                          break;
                          case IntConst: printf("%d ", quad->address_3.value);
                          break;
                          case Temp: printf("t%d ", quad->address_3.value);
                          default: break;
                        }
                        printf("= ");

                      switch (quad->address_1.kind) {
                        case String: printf("%s ", quad->address_1.name);
                          break;
                        case IntConst: printf("%d ", quad->address_1.value);
                          break;
                        case Temp: printf("t%d ", quad->address_1.value);
                        default: break;
                      }

                      printf("<= ");
                      switch (quad->address_2.kind) {
                        case String: printf("%s ", quad->address_2.name);
                          break;
                        case IntConst: printf("%d ", quad->address_2.value);
                          break;
                        case Temp: printf("t%d ", quad->address_2.value);
                        default: break;
                      }
                      printf("\n");
      break;
    case HT:
      if(c0){
        if(c0->kind.stmt == VarK){
          strcpy(quad->address_1.name, c0->attr.name);
          quad->address_1.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_1.name);
        }else if (c0->kind.exp == ConstK) {
          quad->address_1.value = c0->attr.value;
          quad->address_1.kind = IntConst;
          // printf("%d\n", quad->address_1.value);
        }else{
          generate_intermediate_code(quad_list, c0, table);
          // printf("%d\n", temporary);
          quad->address_1.value = temporary;
          quad->address_1.kind = Temp;
          // temporary++;
        }
      }

      if(c1){
        if(c1->kind.stmt == VarK){
          strcpy(quad->address_2.name, c1->attr.name);
          quad->address_2.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_2.name);
        }else if (c1->kind.exp == ConstK) {
          quad->address_2.value = c1->attr.value;
          quad->address_2.kind = IntConst;
          // printf("%d\n", quad->address_2.value);
        }else{
          generate_intermediate_code(quad_list, c1, table);
          // printf("%d\n", temporary);
          quad->address_2.value = temporary;
          quad->address_2.kind = Temp;
          // temporary++;
        }
      }
      temporary++;
      quad->address_3.value = temporary;
      quad->address_3.kind = Temp;

      quad->op = GtrK;
      insert_quadruple(quad_list, quad);
      store_quadruple(quad->op, quad->address_1.kind, quad->address_2.kind, quad->address_3.kind,
                      quad->address_1.value, quad->address_2.value, quad->address_3.value,
                      quad->address_1.name, quad->address_2.name, quad->address_3.name);
                      switch (quad->address_3.kind) {
                        case String: printf("%s ", quad->address_3.name);
                          break;
                          case IntConst: printf("%d ", quad->address_3.value);
                          break;
                          case Temp: printf("t%d ", quad->address_3.value);
                          default: break;
                        }
                        printf("= ");

                      switch (quad->address_1.kind) {
                        case String: printf("%s ", quad->address_1.name);
                          break;
                        case IntConst: printf("%d ", quad->address_1.value);
                          break;
                        case Temp: printf("t%d ", quad->address_1.value);
                        default: break;
                      }

                      printf("> ");
                      switch (quad->address_2.kind) {
                        case String: printf("%s ", quad->address_2.name);
                          break;
                        case IntConst: printf("%d ", quad->address_2.value);
                          break;
                        case Temp: printf("t%d ", quad->address_2.value);
                        default: break;
                      }
                      printf("\n");
      break;
    case HET:
      if(c0){
        if(c0->kind.stmt == VarK){
          strcpy(quad->address_1.name, c0->attr.name);
          quad->address_1.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_1.name);
        }else if (c0->kind.exp == ConstK) {
          quad->address_1.value = c0->attr.value;
          quad->address_1.kind = IntConst;
          // printf("%d\n", quad->address_1.value);
        }else{
          generate_intermediate_code(quad_list, c0, table);
          // printf("%d\n", temporary);
          quad->address_1.value = temporary;
          quad->address_1.kind = Temp;
          // temporary++;
        }
      }

      if(c1){
        if(c1->kind.stmt == VarK){
          strcpy(quad->address_2.name, c1->attr.name);
          quad->address_2.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_2.name);
        }else if (c1->kind.exp == ConstK) {
          quad->address_2.value = c1->attr.value;
          quad->address_2.kind = IntConst;
          // printf("%d\n", quad->address_2.value);
        }else{
          generate_intermediate_code(quad_list, c1, table);
          // printf("%d\n", temporary);
          quad->address_2.value = temporary;
          quad->address_2.kind = Temp;
          // temporary++;
        }
      }
      temporary++;
      quad->address_3.value = temporary;
      quad->address_3.kind = Temp;

      quad->op = GeqK;
      insert_quadruple(quad_list, quad);
      store_quadruple(quad->op, quad->address_1.kind, quad->address_2.kind, quad->address_3.kind,
                      quad->address_1.value, quad->address_2.value, quad->address_3.value,
                      quad->address_1.name, quad->address_2.name, quad->address_3.name);
                      switch (quad->address_3.kind) {
                        case String: printf("%s ", quad->address_3.name);
                          break;
                          case IntConst: printf("%d ", quad->address_3.value);
                          break;
                          case Temp: printf("t%d ", quad->address_3.value);
                          default: break;
                        }
                        printf("= ");

                      switch (quad->address_1.kind) {
                        case String: printf("%s ", quad->address_1.name);
                          break;
                        case IntConst: printf("%d ", quad->address_1.value);
                          break;
                        case Temp: printf("t%d ", quad->address_1.value);
                        default: break;
                      }

                      printf(">= ");
                      switch (quad->address_2.kind) {
                        case String: printf("%s ", quad->address_2.name);
                          break;
                        case IntConst: printf("%d ", quad->address_2.value);
                          break;
                        case Temp: printf("t%d ", quad->address_2.value);
                        default: break;
                      }
                      printf("\n");
      break;
    default:
      break;
  }
}

static void generate_arithop(list_quadruple *quad_list, TreeNode *tree, TipoLista *table){
  TreeNode *c0, *c1, *c2;
  c0 = tree->child[0];
  c1 = tree->child[1];
  c2 = tree->child[2];

  quadruple *quad = malloc(sizeof(quadruple));

  quad->address_1.kind = Empty;
  quad->address_2.kind = Empty;
  quad->address_3.kind = Empty;

  switch (tree->attr.oprtr) {
    case PLUS:
      if(c0){
        if(c0->kind.stmt == VarK){
          strcpy(quad->address_1.name, c0->attr.name);
          quad->address_1.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_1.name);
        }else if (c0->kind.exp == ConstK) {
          quad->address_1.value = c0->attr.value;
          quad->address_1.kind = IntConst;
          // printf("%d\n", quad->address_1.value);
        }else{
          generate_intermediate_code(quad_list, c0, table);
          // printf("%d\n", temporary);
          quad->address_1.value = temporary;
          quad->address_1.kind = Temp;
          // temporary++;
        }
      }

      if(c1){
        if(c1->kind.stmt == VarK){
          strcpy(quad->address_2.name, c1->attr.name);
          quad->address_2.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_2.name);
        }else if (c1->kind.exp == ConstK) {
          quad->address_2.value = c1->attr.value;
          quad->address_2.kind = IntConst;
          // printf("%d\n", quad->address_2.value);
        }else{
          generate_intermediate_code(quad_list, c1, table);
          // printf("%d\n", temporary);
          quad->address_2.value = temporary;
          quad->address_2.kind = Temp;
          // temporary++;
        }
      }


      temporary++;
      quad->address_3.value = temporary;
      quad->address_3.kind = Temp;

      quad->op = AddK;
      insert_quadruple(quad_list, quad);
      store_quadruple(quad->op, quad->address_1.kind, quad->address_2.kind, quad->address_3.kind,
                      quad->address_1.value, quad->address_2.value, quad->address_3.value,
                      quad->address_1.name, quad->address_2.name, quad->address_3.name);
                      switch (quad->address_3.kind) {
                        case String: printf("%s ", quad->address_3.name);
                          break;
                          case IntConst: printf("%d ", quad->address_3.value);
                          break;
                          case Temp: printf("t%d ", quad->address_3.value);
                          default: break;
                        }
                        printf("= ");

                      switch (quad->address_1.kind) {
                        case String: printf("%s ", quad->address_1.name);
                          break;
                        case IntConst: printf("%d ", quad->address_1.value);
                          break;
                        case Temp: printf("t%d ", quad->address_1.value);
                        default: break;
                      }

                      printf("+ ");
                      switch (quad->address_2.kind) {
                        case String: printf("%s ", quad->address_2.name);
                          break;
                        case IntConst: printf("%d ", quad->address_2.value);
                          break;
                        case Temp: printf("t%d ", quad->address_2.value);
                        default: break;
                      }
                      printf("\n");
      break;
    case MINUS:
      if(c0){
        if(c0->kind.stmt == VarK){
          strcpy(quad->address_1.name, c0->attr.name);
          quad->address_1.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_1.name);
        }else if (c0->kind.exp == ConstK) {
          quad->address_1.value = c0->attr.value;
          quad->address_1.kind = IntConst;
          // printf("%d\n", quad->address_1.value);
        }else{
          generate_intermediate_code(quad_list, c0, table);
          // printf("t%d\n", temporary);
          quad->address_1.value = temporary;
          quad->address_1.kind = Temp;
          // temporary++;
        }
      }

      if(c1){
        if(c1->kind.stmt == VarK){
          strcpy(quad->address_2.name, c1->attr.name);
          quad->address_2.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_2.name);
        }else if (c1->kind.exp == ConstK) {
          quad->address_2.value = c1->attr.value;
          quad->address_2.kind = IntConst;
          // printf("%d\n", quad->address_2.value);
        }else{
          generate_intermediate_code(quad_list, c1, table);
          // printf("t%d\n", temporary);
          quad->address_2.value = temporary;
          quad->address_2.kind = Temp;
          // temporary++;
        }
      }
      temporary++;
      quad->address_3.value = temporary;
      quad->address_3.kind = Temp;

      quad->op = SubK;
      insert_quadruple(quad_list, quad);
      store_quadruple(quad->op, quad->address_1.kind, quad->address_2.kind, quad->address_3.kind,
                      quad->address_1.value, quad->address_2.value, quad->address_3.value,
                      quad->address_1.name, quad->address_2.name, quad->address_3.name);
                      switch (quad->address_3.kind) {
                        case String: printf("%s ", quad->address_3.name);
                          break;
                          case IntConst: printf("%d ", quad->address_3.value);
                          break;
                          case Temp: printf("t%d ", quad->address_3.value);
                          default: break;
                        }
                        printf("= ");

                      switch (quad->address_1.kind) {
                        case String: printf("%s ", quad->address_1.name);
                          break;
                        case IntConst: printf("%d ", quad->address_1.value);
                          break;
                        case Temp: printf("t%d ", quad->address_1.value);
                        default: break;
                      }

                      printf("- ");
                      switch (quad->address_2.kind) {
                        case String: printf("%s ", quad->address_2.name);
                          break;
                        case IntConst: printf("%d ", quad->address_2.value);
                          break;
                        case Temp: printf("t%d ", quad->address_2.value);
                        default: break;
                      }
                      printf("\n");
      break;
    case TIMES:
      if(c0){
        if(c0->kind.stmt == VarK){
          strcpy(quad->address_1.name, c0->attr.name);
          quad->address_1.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_1.name);
        }else if (c0->kind.exp == ConstK) {
          quad->address_1.value = c0->attr.value;
          quad->address_1.kind = IntConst;
          // printf("%d\n", quad->address_1.value);
        }else{
          generate_intermediate_code(quad_list, c0, table);
          // printf("t%d\n", temporary);
          quad->address_1.value = temporary;
          quad->address_1.kind = Temp;
          // temporary++;
        }
      }

      if(c1){
        if(c1->kind.stmt == VarK){
          strcpy(quad->address_2.name, c1->attr.name);
          quad->address_2.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_2.name);
        }else if (c1->kind.exp == ConstK) {
          quad->address_2.value = c1->attr.value;
          quad->address_2.kind = IntConst;
          // printf("%d\n", quad->address_2.value);
        }else{
          generate_intermediate_code(quad_list, c1, table);
          // printf("t%d\n", temporary);
          quad->address_2.value = temporary;
          quad->address_2.kind = Temp;
          // temporary++;
        }
      }
      temporary++;
      quad->address_3.value = temporary;
      quad->address_3.kind = Temp;

      quad->op = TimK;
      insert_quadruple(quad_list, quad);
      store_quadruple(quad->op, quad->address_1.kind, quad->address_2.kind, quad->address_3.kind,
                      quad->address_1.value, quad->address_2.value, quad->address_3.value,
                      quad->address_1.name, quad->address_2.name, quad->address_3.name);
                      switch (quad->address_3.kind) {
                        case String: printf("%s ", quad->address_3.name);
                          break;
                          case IntConst: printf("%d ", quad->address_3.value);
                          break;
                          case Temp: printf("t%d ", quad->address_3.value);
                          default: break;
                        }
                        printf("= ");

                      switch (quad->address_1.kind) {
                        case String: printf("%s ", quad->address_1.name);
                          break;
                        case IntConst: printf("%d ", quad->address_1.value);
                          break;
                        case Temp: printf("t%d ", quad->address_1.value);
                        default: break;
                      }

                      printf("* ");
                      switch (quad->address_2.kind) {
                        case String: printf("%s ", quad->address_2.name);
                          break;
                        case IntConst: printf("%d ", quad->address_2.value);
                          break;
                        case Temp: printf("t%d ", quad->address_2.value);
                        default: break;
                      }
                      printf("\n");
      break;
    case OVER:
      if(c0){
        if(c0->kind.stmt == VarK){
          strcpy(quad->address_1.name, c0->attr.name);
          quad->address_1.kind = String;
          strcpy(quad->scope, tree->scope);
          // printf("%s\n", quad->address_1.name);
        }else if (c0->kind.exp == ConstK) {
          quad->address_1.value = c0->attr.value;
          quad->address_1.kind = IntConst;
          // printf("%d\n", quad->address_1.value);
        }else{
          generate_intermediate_code(quad_list, c0, table);
          // printf("t%d\n", temporary);
          quad->address_1.value = temporary;
          quad->address_1.kind = Temp;
          // temporary++;
        }
      }

      if(c1){
        if(c1->kind.stmt == VarK){
          strcpy(quad->address_2.name, c1->attr.name);
          quad->address_2.kind = String;
          strcpy(quad->scope, tree->scope);

          // printf("%s\n", quad->address_2.name);
        }else if (c1->kind.exp == ConstK) {
          quad->address_2.value = c1->attr.value;
          quad->address_2.kind = IntConst;
          // printf("%d\n", quad->address_2.value);
        }else{
          generate_intermediate_code(quad_list, c1, table);
          // printf("t%d\n", temporary);
          quad->address_2.value = temporary;
          quad->address_2.kind = Temp;
          // temporary++;
        }
      }
      temporary++;
      quad->address_3.value = temporary;
      quad->address_3.kind = Temp;

      quad->op = OvrK;
      insert_quadruple(quad_list, quad);
      store_quadruple(quad->op, quad->address_1.kind, quad->address_2.kind, quad->address_3.kind,
                      quad->address_1.value, quad->address_2.value, quad->address_3.value,
                      quad->address_1.name, quad->address_2.name, quad->address_3.name);
                      switch (quad->address_3.kind) {
                        case String: printf("%s ", quad->address_3.name);
                          break;
                          case IntConst: printf("%d ", quad->address_3.value);
                          break;
                          case Temp: printf("t%d ", quad->address_3.value);
                          default: break;
                        }
                        printf("= ");

                      switch (quad->address_1.kind) {
                        case String: printf("%s ", quad->address_1.name);
                          break;
                        case IntConst: printf("%d ", quad->address_1.value);
                          break;
                        case Temp: printf("t%d ", quad->address_1.value);
                        default: break;
                      }

                      printf("/ ");
                      switch (quad->address_2.kind) {
                        case String: printf("%s ", quad->address_2.name);
                          break;
                        case IntConst: printf("%d ", quad->address_2.value);
                          break;
                        case Temp: printf("t%d ", quad->address_2.value);
                        default: break;
                      }
                      printf("\n");
      break;
    default:
      break;
  }
}


static void generate_statement(list_quadruple *quad_list, TreeNode *tree, TipoLista *table) {
  TreeNode *c0, *c1, *c2;
  c0 = tree->child[0];
  c1 = tree->child[1];
  c2 = tree->child[2];
  int aux;
  int aux2;
  int aux3;

  quadruple *quad0 = malloc(sizeof(quadruple));
  quadruple *quad1 = malloc(sizeof(quadruple));
  quadruple *quad2 = malloc(sizeof(quadruple));
  quadruple *quad3 = malloc(sizeof(quadruple));

  quad0->address_1.kind = Empty;
  quad0->address_2.kind = Empty;
  quad0->address_3.kind = Empty;

  quad1->address_1.kind = Empty;
  quad1->address_2.kind = Empty;
  quad1->address_3.kind = Empty;

  quad2->address_1.kind = Empty;
  quad2->address_2.kind = Empty;
  quad2->address_3.kind = Empty;

  quad3->address_1.kind = Empty;
  quad3->address_2.kind = Empty;
  quad3->address_3.kind = Empty;

  switch (tree->kind.stmt) {
    case IfK:
      // if
      if(c0){
        generate_intermediate_code(quad_list, c0, table);
        aux = temporary;
        aux2 = label;
        label++;
        printf("If_false t%d goto L%d\n", aux, aux2);
        quad0->address_1.kind = Temp;
        quad0->address_1.value = aux;
        quad0->address_2.kind = Empty;
        quad0->address_3.kind = LabAddr;
        quad0->address_3.value = aux2;
        quad0->op = IffK;
        insert_quadruple(quad_list, quad0);
        store_quadruple(quad0->op, quad0->address_1.kind, quad0->address_2.kind, quad0->address_3.kind,
                        quad0->address_1.value, quad0->address_2.value, quad0->address_3.value,
                        quad0->address_1.name, quad0->address_2.name, quad0->address_3.name);

        //add quad0
      }
      // then
      if(c1)
        generate_intermediate_code(quad_list, c1, table);
      if (c2) {
        aux3 = label;
        label++;
        printf("goto L%d\n", aux3);
        quad1->address_1.kind = Empty;
        quad1->address_2.kind = Empty;
        quad1->address_3.kind = LabAddr;
        quad1->address_3.value = aux3;
        quad1->op = GtoK;
        insert_quadruple(quad_list, quad1);
        store_quadruple(quad1->op, quad1->address_1.kind, quad1->address_2.kind, quad1->address_3.kind,
                        quad1->address_1.value, quad1->address_2.value, quad1->address_3.value,
                        quad1->address_1.name, quad1->address_2.name, quad1->address_3.name);

        //add quad1
      }
      printf("L%d: ", aux2);
      quad2->address_1.kind = Empty;
      quad2->address_2.kind = Empty;
      quad2->address_3.kind = LabAddr;
      quad2->address_3.value = aux2;
      quad2->op = LblK;
      insert_quadruple(quad_list, quad2);
      store_quadruple(quad2->op, quad2->address_1.kind, quad2->address_2.kind, quad2->address_3.kind,
                      quad2->address_1.value, quad2->address_2.value, quad2->address_3.value,
                      quad2->address_1.name, quad2->address_2.name, quad2->address_3.name);
      // add quad2
      // else
      if(c2){
        generate_intermediate_code(quad_list, c2, table);
        quad3->address_1.kind = Empty;
        quad3->address_2.kind = Empty;
        quad3->address_3.kind = LabAddr;
        quad3->address_3.value = aux3;
        quad3->op = LblK;
        insert_quadruple(quad_list, quad3);
        store_quadruple(quad3->op, quad3->address_1.kind, quad3->address_2.kind, quad3->address_3.kind,
                        quad3->address_1.value, quad3->address_2.value, quad3->address_3.value,
                        quad3->address_1.name, quad3->address_2.name, quad3->address_3.name);
        // add quad3
        printf("L%d: ", aux3);
      }
      break;
    case WhileK:
      if (c0) {
        aux2 = label;
        label++;
        aux3 = label;
        label++;
        printf("L%d: ", aux2);
        quad0->address_1.kind = Empty;
        quad0->address_2.kind = Empty;
        quad0->address_3.kind = LabAddr;
        quad0->address_3.value = aux2;
        quad0->op = LblK;
        insert_quadruple(quad_list, quad0);
        store_quadruple(quad0->op, quad0->address_1.kind, quad0->address_2.kind, quad0->address_3.kind,
                        quad0->address_1.value, quad0->address_2.value, quad0->address_3.value,
                        quad0->address_1.name, quad0->address_2.name, quad0->address_3.name);

        // add quad0
        generate_intermediate_code(quad_list, c0, table);
        aux = temporary;
        printf("If_false t%d goto L%d\n", aux, aux3);
        quad1->address_1.kind = Temp;
        quad1->address_1.value = aux;
        quad1->address_2.kind = Empty;
        quad1->address_3.kind = LabAddr;
        quad1->address_3.value = aux3;
        quad1->op = IffK;
        insert_quadruple(quad_list, quad1);
        store_quadruple(quad1->op, quad1->address_1.kind, quad1->address_2.kind, quad1->address_3.kind,
                        quad1->address_1.value, quad1->address_2.value, quad1->address_3.value,
                        quad1->address_1.name, quad1->address_2.name, quad1->address_3.name);

        // add quad1
      }
        if(c1)
          generate_intermediate_code(quad_list, c1, table);
        printf("goto L%d\n", aux2);
        quad2->address_1.kind = Empty;
        quad2->address_2.kind = Empty;
        quad2->address_3.kind = LabAddr;
        quad2->address_3.value = aux2;
        quad2->op = GtoK;
        insert_quadruple(quad_list, quad2);
        store_quadruple(quad2->op, quad2->address_1.kind, quad2->address_2.kind, quad2->address_3.kind,
                        quad2->address_1.value, quad2->address_2.value, quad2->address_3.value,
                        quad2->address_1.name, quad2->address_2.name, quad2->address_3.name);
        // add quad2

        printf("L%d: ", aux3);
        quad3->address_1.kind = Empty;
        quad3->address_2.kind = Empty;
        quad3->address_3.kind = LabAddr;
        quad3->address_3.value = aux3;
        quad3->op = LblK;
        insert_quadruple(quad_list, quad3);
        store_quadruple(quad3->op, quad3->address_1.kind, quad3->address_2.kind, quad3->address_3.kind,
                        quad3->address_1.value, quad3->address_2.value, quad3->address_3.value,
                        quad3->address_1.name, quad3->address_2.name, quad3->address_3.name);
        // add quad3
      break;
    case AssignK://PROBLEMS
      // printf("AssignK\n");
      generate_intermediate_code(quad_list, c1, table);

      printf("%s", c0->attr.name);
      quad0->op = AsvK;
      quad0->address_1.kind = Temp;
      quad0->address_1.value = temporary;
      quad0->address_2.kind = Empty;
      quad0->address_3.kind = String;
      strcpy(quad0->address_3.name, c0->attr.name);
      if (c0->child[0]) {
        quad0->op = AsaK;
        TreeNode *g0 = c0->child[0];
        if (g0->kind.exp==ConstK) {
          quad0->address_2.kind = IntConst;
          quad0->address_2.value = g0->attr.value;
          printf("[%d] =", quad0->address_2.value);
        }else if (g0->kind.exp==VarK) {
          quad0->address_2.kind = String;
          strcpy(quad0->address_2.name, g0->attr.name);
          printf("[%s] =", quad0->address_2.name);
          strcpy(quad0->scope, g0->scope);

        }else{
          generate_intermediate_code(quad_list, c0, table);
          aux = temporary;
          quad0->address_2.kind = Temp;
          quad0->address_2.value = aux;
          printf("[t%d] =", quad0->address_2.value);
        }
      }else
        printf(" =");

      // TreeNode *g1 = c1->child[0];
      // if(g1)
      // while (g1->child[0]) {
      //   g1 = g1->child[0];
      // }
      // if(c1->kind.exp == CallK){
      //   generate_intermediate_code(quad_list, c1);
      //   quad0->address_1.kind = Temp;
      //   quad0->address_1.value = temporary;
      //   temporary++;
      //   printf(" t%d\n", quad0->address_1.value);
      // }else if(g1){
      // if(g1->kind.exp = ConstK){
      //     quad0->address_1.kind = IntConst;
      //     quad0->address_1.value = g1->attr.value;
      //     printf(" %d\n", quad0->address_1.value);
      // }else if(g1->kind.exp = VarK){
      //     quad0->address_1.kind = String;
      //     strcpy(quad0->address_1.name, g1->attr.name);
      //     printf(" %s\n", quad0->address_1.name);
      // }
      // }
      // else {
      //   generate_intermediate_code(quad_list, c1);

        printf(" t%d\n", quad0->address_1.value);
      // }
        // quad0->next = NULL;
        insert_quadruple(quad_list, quad0);
        store_quadruple(quad0->op, quad0->address_1.kind, quad0->address_2.kind, quad0->address_3.kind,
                        quad0->address_1.value, quad0->address_2.value, quad0->address_3.value,
                        quad0->address_1.name, quad0->address_2.name, quad0->address_3.name);
      break;
    case ReturnK:
      // printf("ReturnK\n");
      quad0->address_1.kind = Empty;
      quad0->address_2.kind = Empty;
      if (!c0) {
        printf("Return void\n");
        quad0->address_3.kind = Empty;
      }else{
        TreeNode *g1 = c0->child[0];
        if(g1){
          generate_intermediate_code(quad_list, c0, table);
          aux = temporary;
          printf("Return t%d\n", aux);
          quad0->address_3.kind = Temp;
          quad0->address_3.value = aux;
        }else{
          if (c0->kind.stmt == ConstK) {
            printf("Return %d\n", c0->attr.value);
            quad0->address_3.kind = IntConst;
            quad0->address_3.value = c0->attr.value;
          }else{
            printf("Return %s\n", c0->attr.name);
            quad0->address_3.kind = String;
            strcpy(quad0->address_3.name, c0->attr.name);
          }
        }
      }
      quad0->op = RetK;
      insert_quadruple(quad_list, quad0);
      store_quadruple(quad0->op, quad0->address_1.kind, quad0->address_2.kind, quad0->address_3.kind,
                      quad0->address_1.value, quad0->address_2.value, quad0->address_3.value,
                      quad0->address_1.name, quad0->address_2.name, quad0->address_3.name);
      // add quad0
      break;
    case VarK:
      // printf("Vark\n");
      // not used in intermediate code generation
      break;
    case VecK:
      // printf("Veck\n");
      // not used in intermediate code generation
      break;
    case FuncK:
      // printf("FuncK\n");
      printf("%s: ", tree->attr.name);
      quad0->address_1.kind = Empty;
      quad0->address_2.kind = Empty;
      quad0->address_3.kind = String;
      strcpy(quad0->address_3.name, tree->attr.name);
      quad0->op = LblK;
      insert_quadruple(quad_list, quad0);
      store_quadruple(quad0->op, quad0->address_1.kind, quad0->address_2.kind, quad0->address_3.kind,
                      quad0->address_1.value, quad0->address_2.value, quad0->address_3.value,
                      quad0->address_1.name, quad0->address_2.name, quad0->address_3.name);

      // add quad0
      if (c0) {
        index_parameters = 1;
        generate_intermediate_code(quad_list, c0, table);
      }
      if (c1) {
        generate_intermediate_code(quad_list, c1, table);
      }
      if(!strcmp(tree->attr.name, "main")){
        break;
      }
      quad1->address_1.kind = Empty;
      quad1->address_2.kind = Empty;
      quad1->address_3.kind = String;
      strcpy(quad1->address_3.name, tree->attr.name);
      quad1->op = EofK;
      insert_quadruple(quad_list, quad1);
      break;
    case FuncVecK:
      strcpy(quad0->address_1.name, tree->attr.name);
      quad0->address_1.kind = String;
      strcpy(quad0->address_2.name, tree->scope);
      quad0->address_2.kind = String;
      quad0->address_3.value = index_parameters;
      quad0->address_3.kind = IntConst;
      quad0->op = PrmArrK;
      insert_quadruple(quad_list, quad0);
      index_parameters++;
      break;
    case FuncVarK:
      strcpy(quad0->address_1.name, tree->attr.name);
      quad0->address_1.kind = String;
      strcpy(quad0->address_2.name, tree->scope);
      quad0->address_2.kind = String;
      quad0->address_3.value = index_parameters;
      quad0->address_3.kind = IntConst;
      quad0->op = PrmVarK;
      insert_quadruple(quad_list, quad0);
      index_parameters++;
      break;
    default:
    printf("Oops: uncomment something!\n");
      break;
  }
}

static void generate_expression(list_quadruple *quad_list, TreeNode *tree, TipoLista *table) {
  TreeNode *c0, *c1;
  c0 = tree->child[0];
  c1 = tree->child[1];
  int count;
  int i;
  int aux = 0;

  quadruple *quad0 = malloc(sizeof(quadruple));
  quadruple *quad1 = malloc(sizeof(quadruple));
  quadruple *quad2 = malloc(sizeof(quadruple));

  quadruple *quad_aux[9];

  for (i = 0; i < 9; i++) {
    quad_aux[i] = malloc(sizeof(quadruple));
  }

  quad0->address_1.kind = Empty;
  quad0->address_2.kind = Empty;
  quad0->address_3.kind = Empty;

  quad1->address_1.kind = Empty;
  quad1->address_2.kind = Empty;
  quad1->address_3.kind = Empty;

  quad2->address_1.kind = Empty;
  quad2->address_2.kind = Empty;
  quad2->address_3.kind = Empty;

  switch (tree->kind.exp) {
    case TypeK:
      generate_intermediate_code(quad_list, c0, table);
      break;
    case RelOpK:
      generate_relop(quad_list, tree, table);//print nothing
      break;
    case ArithOpK:
      generate_arithop(quad_list, tree, table);
      break;
    case ConstK:
      quad0->address_1.value = tree->attr.value;
      quad0->address_1.kind = IntConst;
      quad0->address_3.kind = Temp;
      quad0->address_3.value = temporary+1;
      quad0->op = CstK;
      insert_quadruple(quad_list, quad0);
      store_quadruple(quad0->op, quad0->address_1.kind, quad0->address_2.kind, quad0->address_3.kind,
                      quad0->address_1.value, quad0->address_2.value, quad0->address_3.value,
                      quad0->address_1.name, quad0->address_2.name, quad0->address_3.name);
      temporary++;
      printf("t%d = %d\n", quad0->address_3.value, quad0->address_1.value);
      break;
    case IdK:
    strcpy(quad0->address_1.name, tree->attr.name);
    quad0->address_1.kind = String;
    quad0->address_3.kind = Temp;
    quad0->address_3.value = temporary+1;
    quad0->op = VstK;
    insert_quadruple(quad_list, quad0);
    store_quadruple(quad0->op, quad0->address_1.kind, quad0->address_2.kind, quad0->address_3.kind,
                    quad0->address_1.value, quad0->address_2.value, quad0->address_3.value,
                    quad0->address_1.name, quad0->address_2.name, quad0->address_3.name);
    temporary++;
    printf("t%d = %s\n", quad0->address_3.value, quad0->address_1.name);
      break;
    case VecIndexK:
    strcpy(quad0->address_1.name, tree->attr.name);
    quad0->address_1.kind = String;

    if (c0->kind.exp==ConstK) {
      quad0->address_2.kind = IntConst;
      quad0->address_2.value = c0->attr.value;
    }else if (c0->kind.exp==VarK) {
      quad0->address_2.kind = String;
      strcpy(quad0->address_2.name, c0->attr.name);
      strcpy(quad0->scope, tree->scope);

    }else{
      generate_intermediate_code(quad_list, c0, table);
      aux = temporary;
      quad0->address_2.kind = Temp;
      quad0->address_2.value = aux;
    }

    quad0->address_3.kind = Temp;
    quad0->address_3.value = temporary+1;
    quad0->op = AstK;
    insert_quadruple(quad_list, quad0);
    store_quadruple(quad0->op, quad0->address_1.kind, quad0->address_2.kind, quad0->address_3.kind,
                    quad0->address_1.value, quad0->address_2.value, quad0->address_3.value,
                    quad0->address_1.name, quad0->address_2.name, quad0->address_3.name);
    temporary++;
    switch (c0->kind.exp) {
      case ConstK:
        printf("t%d = %s[%d]\n", quad0->address_3.value, quad0->address_1.name, quad0->address_2.value);
        break;
      case VarK:
        printf("t%d = %s[%s]\n", quad0->address_3.value, quad0->address_1.name, quad0->address_2.name);
        break;
      default:
        printf("t%d = %s[t%d]\n", quad0->address_3.value, quad0->address_1.name, quad0->address_2.value);
        break;
      }


      break;
    case CallK:

      i=0;

      count = 0;
      quad0->address_3.kind = String;
      strcpy(quad0->address_3.name, tree->attr.name);
      if (!strcmp(quad0->address_3.name, "input")) {
        quad0->op = InnK;
      }else{
        quad0->op = CalK;
      }

      if (!c0) {
        quad0->address_1.kind = Empty;
        if (check_if_int(table, quad0->address_3.name)) {
          temporary++;
          quad0->address_2.kind = Temp;
          quad0->address_2.value = temporary;
        }else{
          quad0->address_2.kind = Empty;
        }

        insert_quadruple(quad_list, quad0);

        printf("t%d = call %s, 0\n", temporary, tree->attr.name);
        // store_quadruple(quad0->op, quad0->address_1.kind, quad0->address_2.kind, quad0->address_3.kind,
                        // quad0->address_1.value, quad0->address_2.value, quad0->address_3.value,
                        // quad0->address_1.name, quad0->address_2.name, quad0->address_3.name);
        break;
      }

      do{

              if (c0->child[0]) {
                int aux;

                generate_children_code(quad_list, c0, table);

                quad0 = malloc(sizeof(quadruple));
                quad0->address_1.kind = Empty;
                quad0->address_2.kind = Empty;
                quad0->address_3.kind = Empty;

                aux = temporary;
                printf("param t%d\n", aux);
                quad0->address_1.kind = Empty;
                quad0->address_2.kind = Empty;
                quad0->address_3.kind = Temp;
                quad0->address_3.value = aux;
              }else if(!c0->child[0]){
                switch (c0->kind.exp) {
                  case ConstK:
                    printf("param %d\n", c0->attr.value);
                    quad0->address_1.kind = Empty;
                    quad0->address_2.kind = Empty;
                    quad0->address_3.kind = IntConst;
                    quad0->address_3.value = c0->attr.value;
                    break;
                  case VarK:
                    printf("param %s\n", c0->attr.name);
                    quad0->address_1.kind = Empty;
                    quad0->address_2.kind = Empty;
                    quad0->address_3.kind = String;
                    strcpy(quad0->address_3.name, c0->attr.name);
                    strcpy(quad0->scope, tree->scope);
                    break;
                }
              }
              count++;
                c0 = c0->sibling;

                quad0->op = PrmK;
                insert_quadruple(quad_list, quad0);
                store_quadruple(quad0->op, quad0->address_1.kind, quad0->address_2.kind, quad0->address_3.kind,
                                quad0->address_1.value, quad0->address_2.value, quad0->address_3.value,
                                quad0->address_1.name, quad0->address_2.name, quad0->address_3.name);
              // add quad0
            }while (c0);

            quad1->address_3.kind = String;
            strcpy(quad1->address_3.name, tree->attr.name);
            if (check_if_int(table, quad1->address_3.name)) {
              temporary++;
              quad1->address_2.kind = Temp;
              quad1->address_2.value = temporary;
            }else{
              quad1->address_2.kind = Empty;
            }
              quad1->address_1.kind = IntConst;
              quad1->address_1.value = count;
              // quad1->address_2.kind = Temp;
              // quad1->address_2.value = temporary;

              if (!strcmp(quad1->address_3.name,"output")) {
                quad1->op = OutK;
              }else{
                quad1->op = CalK;
              }

              insert_quadruple(quad_list, quad1);
              store_quadruple(quad1->op, quad1->address_1.kind, quad1->address_2.kind, quad1->address_3.kind,
                              quad1->address_1.value, quad1->address_2.value, quad1->address_3.value,
                              quad1->address_1.name, quad1->address_2.name, quad1->address_3.name);


        printf("t%d = call %s, %d\n", temporary, tree->attr.name, count);
    //   }
    //

    //
    //   // add quad1
      break;
    // }
    default:
      break;
  }
}

void generate_intermediate_code(list_quadruple *quad_list, TreeNode *tree, TipoLista *table){
  if(tree!=NULL){
    switch (tree->nodekind) {
      case StmtK:
        // printf("statement\n");
        generate_statement(quad_list, tree, table);
        break;
      case ExpK:
        // printf("expression\n");
        generate_expression(quad_list, tree, table);
        break;
      default:
        break;
    }
    generate_intermediate_code(quad_list, tree->sibling, table);
  }
}

void generate_children_code(list_quadruple *quad_list, TreeNode *tree, TipoLista *table){
  if(tree!=NULL){
    switch (tree->nodekind) {
      case StmtK:
        // printf("statement\n");
        generate_statement(quad_list, tree, table);
        break;
      case ExpK:
        // printf("expression\n");
        generate_expression(quad_list, tree, table);
        break;
      default:
        break;
    }
  }
}

void generate_icode_launcher(list_quadruple *quad_list, TreeNode *tree, TipoLista *vetor){
    int i;
    file_quadruples = fopen("file_quadruples.txt", "w");
    quad_list->start = NULL;

    quadruple *quad = malloc(sizeof(quadruple));
    quad->address_1.kind = Empty;
    quad->address_2.kind = Empty;
    quad->address_3.kind = Empty;
    quad->op = NopK;
    insert_quadruple(quad_list, quad);

    quad->address_3.kind = String;
    strcpy(quad->address_3.name, "main");
    quad->op = GtoK;
    insert_quadruple(quad_list, quad);

    generate_intermediate_code(quad_list, tree, vetor);
    quad->address_3.kind = Empty;
    quad->op = HltK;
    insert_quadruple(quad_list, quad);

    fclose( file_quadruples );



    add_indexes_to_table(vetor, quad_list);
    add_parameters_to_table(vetor, quad_list);
    printf("\n\n\n\n\n\n\n\n");
    //
    print_quadruple_list(quad_list);

    // file_read_quadruples = fopen("file_quadruples.txt", "r");
    // quad_list->start = NULL;
    // printf("\n\n\n\n\n\n\n\n");
    // insert_quadruples(quad_list);
    // print_quadruple_list(quad_list);
    // fclose( file_read_quadruples );
}
