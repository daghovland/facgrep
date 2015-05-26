/* regexp.c

   Copyright 2008 University of Bergen

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA  02110-1301, USA */

/*   Written July, 2008 by Dag Hovland  */

#include "regexp.h"


follow_set** get_prec(regexp*);
size_t* get_size_prec(regexp*);
follow_set** get_follow(regexp*);
size_t * get_size_follow(regexp*);
bool counter_unambiguous_gen(regexp*, size_t, regexp**, follow_set ** (*) (regexp*), size_t * (*) (regexp*));

void app_position(re_pos*, size_t, re_pos*);
re_pos* copy_position(re_pos*, re_pos*);
size_t copy_first_set(regexp**, regexp*);
size_t copy_last_set(regexp**, regexp*);
size_t copy_follow_set(follow_set**, regexp*, size_t);
/* Total number of positions in the regular expressions */
size_t num_pos;
/* Keeps track of whether correct ordering of initialization is followed */
size_t status_order;




size_t regexp_size;
size_t re_pos_size;
size_t size_follow_size;
size_t first_last_set_size;
size_t follow_set_size;
size_t updates_size;
size_t constraints_r_size;
size_t fac_state_size;
size_t num_epsilons;

size_t cur_alph_len;

size_t max_pos;
size_t max_pos_length(regexp*);




/* Buffer with memory for all the regexp structs needed */
regexp* re_buf;
size_t re_buf_pos;
size_t re_buf_size;

regexp* get_regexp(void);


/**
   Reutrns the position in the alphabet of the letter
 **/
size_t getCharAlphPos(char letter){
  assert(128 + (int) letter >= 0 && 128 + (int) letter < MAX_CHAR_VAL);
  assert(charAlphPos[128 + (int) letter] < cur_alph_len);
  return charAlphPos[128 + (int) letter];
}

/**
   Sets the position in the alphabet of the letter
 **/
void addCharAlphPos(char letter){
  assert(charAlphPos[letter] == UNUSED_CHAR);
  assert(128 + (int) letter >= 0 && 128 + (int) letter < MAX_CHAR_VAL);
  ++alph_size;
  if (alph_size >= cur_alph_len){
    cur_alph_len *= 2;
    alphabet = realloc(alphabet, cur_alph_len);
  }
  charAlphPos[128 + (int) letter] = alph_size;
  alphabet[alph_size] = letter;
}


/**
   Initializes important variables
   and some memory structures
   Takes the length of the regular expression as argument
   Must be called before any of the create_ functions
 **/
void pre_parse_init(size_t rlen){

  num_pos_letters = 0;
  num_epsilons = 0;
  num_pos = 0;
  cur_pos_letters = 0;
  num_constraints = 0;

  re_len = (rlen == 0) ? (size_t) 1 : rlen;
  re_buf_size = 2 * re_len;

  re_buf = (regexp*) calloc(re_buf_size, sizeof(regexp));
  charAlphPos = (size_t*) calloc(MAX_CHAR_VAL, sizeof(size_t) );
  cur_alph_len = re_len + 1;
  alphabet = calloc(cur_alph_len, 1);

  if(!re_buf || !charAlphPos || !alphabet){
    fprintf(stderr, "Out of memory.\n");
    exit(EXIT_FAILURE);
  }
  re_buf_pos = 0;
  alph_size = 0;


  status_order = STATUS_PRE_INIT;
  return;
}


void post_parse_init(regexp* result){
  size_t i;
  assert (status_order == STATUS_PRE_INIT);

  max_pos = max_pos_length(result);
  max_pos = (max_pos == 0) ? (size_t) 1 : max_pos;

  re_pos_size = max_pos * sizeof(size_t);
  updates_size = num_constraints + 1;
  follow_set_size = (alph_size+1) * (sizeof(follow_set*) + (num_constraints + 1) * ( num_pos_letters + num_epsilons ) * ( sizeof(follow_set) + updates_size) );
  first_last_set_size = ( num_pos_letters + num_epsilons ) * sizeof(regexp*);
  size_follow_size = (num_constraints+1) * ( num_pos_letters + num_epsilons ) * (alph_size+1) * sizeof(size_t);
  alph_buf_size = ( num_pos_letters + num_epsilons ) + 1 + num_epsilons;
  pos_letters_size = ( num_pos_letters + num_epsilons ) * sizeof(regexp*);
  constraints_r_size = (num_constraints+1) * sizeof(regexp*);
  fac_state_size = sizeof(fac_state) + sizeof(size_t) * (num_constraints+2);

  assert(num_pos_letters + num_epsilons > 0);
  pos_letters = (regexp**) calloc(num_pos_letters + num_epsilons, sizeof(regexp*));
  status_order = STATUS_PRE_MEM_INIT;
  init_regexp_mem(result);
  status_order = STATUS_MEM_INIT;
  init_regexp(result);
  assert(num_constraints == result->num_constraints_r);
  assert(num_constraints == 0 || result->constraints_r[num_constraints-1] != 0);
  for(i = 0; i < result->num_constraints_r; i++)
    result->constraints_r[i]->value.constr.pos_constraints = i;
  assert(num_constraints == result->num_constraints_r);

  status_order = STATUS_POST_INIT;
}


/**
   Sets up all the members that need memory
 **/
void init_regexp_mem(regexp* re){
  assert(status_order == STATUS_PRE_MEM_INIT);
  assert(re->tpos.list == 0);
  re->tpos.list = (size_t*) get_buf(re_pos_size);
  assert(re->constraints_r == 0);
  re->constraints_r = (regexp**) get_buf(constraints_r_size);
  re->size_follow_set = (size_t*) calloc((num_constraints+1) * ( num_pos_letters + num_epsilons ) * (alph_size+1), sizeof(size_t));
  re->size_prec_set = (size_t*) calloc((num_constraints+1) * ( num_pos_letters + num_epsilons ) * (alph_size+1), sizeof(size_t));
  re->follow_set = (follow_set **) calloc(
      (alph_size+1) *
      (
          sizeof(follow_set*)
          + (num_constraints + 1)
          * ( num_pos_letters + num_epsilons )
          * ( sizeof(follow_set) + num_constraints + 1 )
      )
      , 1 );
  re->prec_set = (follow_set **) calloc(
      (alph_size+1) *
      (
          sizeof(follow_set*)
          + (num_constraints + 1)
          * ( num_pos_letters + num_epsilons )
          * ( sizeof(follow_set) + num_constraints + 1 )
      )
      , 1 );

  switch(re->type){
  case RE_CHOICE:
  case RE_CONCAT:
    init_regexp_mem(re->value.re2);
    /*@fallthrough@*/
  case RE_CONSTR:
    init_regexp_mem(re->r1);
    break;
  case RE_ANY_CHAR:
  case RE_LETTER:
  case RE_CHAR_CLASS:
  case RE_EPSILON:
    break;
  default:
    assert(0);
  }
  return;
}





constraint create_constraint(unsigned int l, unsigned int u){
  constraint c;
  c.lower = l;
  c.upper = u;
  c.pos_constraints = -1;
  return c;
}


regexp* create_re_choice(regexp* r1, regexp* r2){
  return create_bin_regexp(r1, r2, RE_CHOICE);
}

regexp* create_re_cat(regexp* r1, regexp* r2){
  return create_bin_regexp(r1, r2, RE_CONCAT);
}

regexp* create_bin_regexp(regexp* r1, regexp* r2, unsigned int type){
  regexp* r = create_node_regexp(r1, type);
  r->value.re2 = r2;
  return r;
}

regexp* create_re_cons(regexp* r1, constraint c){
  regexp* r = create_node_regexp(r1, RE_CONSTR);
  r->value.constr = c;
  num_constraints++;
  return r;
}


regexp* create_re_any_char(){
  regexp* r = create_regexp(RE_ANY_CHAR);
  num_pos_letters++;
  r->value.let_inf.let_val = 128;
  return r;
}
regexp* create_re_char_class(char l, char u){
  char i;
  regexp* r = create_regexp(RE_CHAR_CLASS);
  r->value.let_inf.let_val = l;
  r->value.let_inf.upper = u;
  num_pos_letters++;
  if(l <= u){
    for(i = l; i <= u; i++){
      if(getCharAlphPos(i) == UNUSED_CHAR)
        addCharAlphPos(i);
    }
  }
  return r;
}


regexp* create_re_letter(char l){
  regexp* r = create_regexp(RE_LETTER);
  r->value.let_inf.let_val = l;
  r->value.let_inf.upper = l;
  num_pos_letters++;
  if(getCharAlphPos(l) == UNUSED_CHAR)
    addCharAlphPos(l);
  return r;
}

regexp* create_re_epsilon(){
  num_epsilons++;
  return create_regexp(RE_EPSILON);
}

regexp* create_node_regexp(regexp* r1, unsigned int type){
  regexp* r = create_regexp(type);
  r->r1 = r1;
  return r;
}

regexp* create_regexp(unsigned int type){
  regexp* r = get_regexp();
  assert(status_order == STATUS_PRE_INIT);
  r->type = type;
  r->nullable = 0;
  r->size_first_set = 0;
  r->size_last_set = 0;
  num_pos++;
  return r;
}





regexp* get_regexp(void){
  assert(re_buf_size > re_buf_pos);
  return &(re_buf[re_buf_pos++]);
}

/**
   A kind of "malloc"
   takes necessary amount of the allocated memory
 **/
void* get_buf(size_t chars){
  return calloc(chars, 1);
}



/**
   Returns the maximum length of a position in the regexp
 **/
size_t max_pos_length(regexp* re){
  size_t l1, l2;
  switch(re->type){
  case RE_EPSILON:
  case RE_LETTER:
  case RE_CHAR_CLASS:
  case RE_ANY_CHAR:
    return 1;
  case RE_CONSTR:
    return max_pos_length(re->r1) + 1;
  case RE_CHOICE:
  case RE_CONCAT:
    l1 = max_pos_length(re->r1);
    l2 = max_pos_length(re->value.re2);
    return 1 + ((l1 > l2) ? l1 : l2);
  default:
    assert(0);
  }
  assert(0);
  return 0;
}


/**
   Used by init_regexp to initialize
   choice and concatenation structures
 **/
void bin_init_regexp(regexp* re){
  app_position(&(re->tpos), POS_LEFT, &(re->r1->tpos));
  app_position(&(re->tpos), POS_RIGHT, &(re->value.re2->tpos));
  init_regexp(re->r1);
  init_regexp(re->value.re2);
  re->num_constraints_r = copy_constraints_r(re->constraints_r, re->r1);
  re->num_constraints_r += copy_constraints_r(re->constraints_r + re->num_constraints_r, re->value.re2);
}

/**
   Extracts the alphabet, the set of letters used in the
   regular expression
   Also sets the position indicator for each regular expression
 **/

void init_regexp(regexp* re){
  assert( status_order == STATUS_MEM_INIT );
  switch(re->type){
  case RE_EPSILON:
    re->nullable = 1;
    re->num_constraints_r = 0;
    re->min_word_length = 0;
    break;
  case RE_ANY_CHAR :
  case RE_CHAR_CLASS:
  case RE_LETTER : {
    re->nullable = 0;
    re->num_constraints_r = 0;
    re->min_word_length = 1;
    assert(cur_pos_letters < num_pos_letters);
    re->value.let_inf.pos_letters_pos = cur_pos_letters++;
    pos_letters[re->value.let_inf.pos_letters_pos] = re;
    break;
  }
  case RE_CHOICE:
    bin_init_regexp(re);
    re->nullable = re->r1->nullable || re->value.re2->nullable;
    re->min_word_length = ( re->value.re2->min_word_length > re->r1->min_word_length ) ? re->r1->min_word_length : re->value.re2->min_word_length;
    break;
  case RE_CONCAT: {
    bin_init_regexp(re);
    re->nullable = re->r1->nullable && re->value.re2->nullable;
    re->min_word_length = re->value.re2->min_word_length + re->r1->min_word_length;
    break;
  }
  case RE_CONSTR:
    app_position(&(re->tpos), POS_LEFT, &(re->r1->tpos));
    init_regexp(re->r1);
    re->nullable = re->r1->nullable || (re->value.constr.lower == 0);
    re->num_constraints_r =  copy_constraints_r(re->constraints_r, re->r1);
    re->constraints_r[re->num_constraints_r++] = re;
    re->min_word_length = re->r1->min_word_length * re->value.constr.lower;
    break;
  default:
    assert(false);
    break;
  }
  return;
}

/**
   Multiplies out nested numerical constraints, e.g.: a{1,2}{1,2} -> a{1,4}
 **/
regexp* mult_constr(regexp* re){
  switch(re->type){
  case RE_EPSILON:
  case RE_LETTER:
  case RE_ANY_CHAR:
  case RE_CHAR_CLASS:
    break;
  case RE_CHOICE:
  case RE_CONCAT:
    re->r1 = mult_constr(re->r1);
    re->value.re2 = mult_constr(re->value.re2);
    break;
  case RE_CONSTR:
    re->r1 = mult_constr(re->r1);
    if(re->r1->type == RE_CONSTR
        &&
        (( re->r1->value.constr.upper == 0 )
            || ( re->value.constr.lower >= (re->r1->value.constr.lower - 1)/(re->r1->value.constr.upper -  re->r1->value.constr.lower ) )
        )
    ) {
      re->r1->value.constr.lower *= re->value.constr.lower;
      re->r1->value.constr.upper *= re->value.constr.upper;
      re = re->r1;
      num_constraints--;
    }
    break;
  default:
    assert(false);
  }
  return re;
}
/**
   Removes the forbidden subexpressions \e+\e, r*\e, \e*r, and \e{n,m}
 **/
regexp* simpl_eps(regexp* re){
  switch(re->type){
  case RE_EPSILON:
  case RE_LETTER:
  case RE_CHAR_CLASS:
  case RE_ANY_CHAR:
    break;
  case RE_CHOICE:
    re->r1 = simpl_eps(re->r1);
    re->value.re2 = simpl_eps(re->value.re2);
    if(re->r1->type == RE_EPSILON && re->value.re2->type == RE_EPSILON)
      re->type = RE_EPSILON;
    break;
  case RE_CONCAT:
    re->r1 = simpl_eps(re->r1);
    re->value.re2 = simpl_eps(re->value.re2);
    if(re->r1->type == RE_EPSILON){
      regexp* tmp  = re->value.re2;
      re = tmp;
    } else if(re->value.re2->type == RE_EPSILON){
      regexp* tmp  = re->r1;
      re = tmp;
    }
    break;
  case RE_CONSTR:
    re->r1 = simpl_eps(re->r1);
    if(re->r1->type == RE_EPSILON){
      re->type = RE_EPSILON;
      num_constraints--;
    }
    break;
  default:
    assert(false);
  }
  return re;
}
/**
   Maps nullable expressions to non-nullable expressions
   Is the mapping "red" in article
   Does not map constraints, because this could lead to
   super-pol. memory usage
 **/
bool remove_null(regexp* re){
  assert(re->nullable);
  switch(re->type){
  case RE_EPSILON:
  case RE_LETTER:
  case RE_ANY_CHAR:
  case RE_CHAR_CLASS:
    assert(false);
    break;
  case RE_CHOICE:
    return remove_null(re->r1) && remove_null(re->value.re2);
    break;
  case RE_CONCAT:
    fprintf(stderr, "\nCannot remove epsilon from : ");
    fprint_regexp(stderr, re);
    return false;
  case RE_CONSTR:
    if(re->r1->nullable){
      if(!remove_null(re->r1))
        return false;
      if(re->value.constr.lower == 0)
        re->value.constr.lower = 1;
    } else {
      assert(re->value.constr.lower == 0);
      re->value.constr.lower = 1;
    }
    break;
  default:
    assert(false);
    return false;
  }
  return true;
}

/**
   Uses "remove_null" to translate to constraint normal form
 **/
bool constraint_normal_form(regexp* re){
  switch(re->type){
  case RE_EPSILON:
  case RE_LETTER:
  case RE_ANY_CHAR:
  case RE_CHAR_CLASS:
    return true;
    break;
  case RE_CHOICE:
  case RE_CONCAT:
    return (constraint_normal_form(re->r1) && constraint_normal_form(re->value.re2));
    break;
  case RE_CONSTR:
    if(re->r1->nullable && (re->value.constr.lower != 0 || re->value.constr.upper != 0)){
      if(remove_null(re->r1))
        re->value.constr.lower = 0;
      else
        return false;
      return constraint_normal_form(re->r1);
    }
    return true;
    break;
  default:
    assert(false);
  }
  assert(false);
  return false;
}

/**
   Appends "child" to position pos and inserts this into position dest
 **/
void app_position(re_pos* pos, size_t child, re_pos* dest){
  dest = copy_position(pos, dest);
  dest->nlist++;
  dest->list[pos->nlist] = child;
  return;
}

/**
   Copies position src to position dest
   dest must be initialized with correct size
 **/
re_pos* copy_position(re_pos* src, re_pos* dest){
  assert(re_len > src->nlist);
  memcpy(dest->list, src->list, (sizeof(int)) * src->nlist);
  dest->nlist = src->nlist;
  return dest;
}

/**
   Copies first_set from regexp src to first_set (regexp**) dest
 **/
size_t copy_constraints_r(regexp** dest, regexp* src){
  memcpy(dest, src->constraints_r, (sizeof(regexp*)) * src->num_constraints_r);
  return src->num_constraints_r;
}


/**
   Copies first_set from regexp src to first_set (regexp**) dest
 **/
size_t copy_first_set(regexp** dest, regexp* src){
  memcpy(dest, src->first_set, (sizeof(regexp*)) * src->size_first_set);
  return src->size_first_set;
}

/**
   Copies first_set from regexp src to first_set (regexp**) dest
 **/
size_t copy_last_set(regexp** dest, regexp* src){
  memcpy(dest, src->last_set, (sizeof(regexp*)) * src->size_last_set);
  return src->size_last_set;
}

bool counter_unambiguous_rev(regexp* re){
  return counter_unambiguous_gen(re, re->size_last_set, re->last_set, get_prec, get_size_prec);
}
bool counter_unambiguous(regexp* re){
  return counter_unambiguous_gen(re, re->size_first_set, re->first_set, get_follow, get_size_follow);
}

follow_set** get_prec(regexp* re){
  return re->prec_set;
}

size_t* get_size_prec(regexp* re){
  return re->size_prec_set;
}

follow_set** get_follow(regexp* re){
  return re->follow_set;
}

size_t* get_size_follow(regexp* re){
  return re->size_follow_set;
}

/**
   Returns true if two regexps could be in the same follow set in an unambiguous expression
   Called from counter_unambbiguous_get
   Similar to incomatible ~ in the article.
 **/
bool regexp_unamb(regexp * r1, regexp * r2){
  switch(r1->type){
  case RE_ANY_CHAR:
    return false;
  case RE_CHAR_CLASS:
    switch(r2->type){
    case RE_ANY_CHAR:
      return false;
    case RE_LETTER:
      return r1->value.let_inf.let_val > r2->value.let_inf.let_val
      || r1->value.let_inf.upper < r2->value.let_inf.let_val;
    case RE_CHAR_CLASS:
      return r1->value.let_inf.upper < r2->value.let_inf.let_val
      || r2->value.let_inf.upper < r1->value.let_inf.let_val;
    default:
      assert(false);
    }
    case RE_LETTER:
      switch(r2->type){
      case RE_ANY_CHAR:
        return false;
      case RE_LETTER:
        return r1->value.let_inf.let_val != r2->value.let_inf.let_val;
      case RE_CHAR_CLASS:
        return r1->value.let_inf.upper < r2->value.let_inf.let_val
        || r1->value.let_inf.let_val > r2->value.let_inf.let_val;
      default:
        assert(false);
      }
      default:
        assert(false);
  }
  assert(false);
  return false;
}

/**
   Tests that the last and prec sets are counter-unambiguous (Whether the reverse is counter-unambiguous)
 **/
bool counter_unambiguous_gen(regexp* re, size_t size_set, regexp** set, follow_set ** (*get_set) (regexp*), size_t * (*get_size_set) (regexp*)){
  size_t i, j, rn, cn, con;
  bool unambiguous = true;
  for(i = 0; i < size_set; i++){
    for(j = 0; j < i; j++){
      if( ! regexp_unamb(set[i], set[j])){
        unambiguous = false;
        break;
      }
    }
    if(!unambiguous)
      break;
  }
  if(unambiguous){
    for(rn = 0; rn < num_pos_letters; rn++){
      regexp * q = pos_letters[rn];
      for(cn = 0; cn <= alph_size; cn++){
        for(i = 0; i < get_size_set(q)[cn]; i++){
          follow_set follqci = get_set(q)[cn][i];
          regexp * qi = (regexp*) follqci.next;
          for(j = 0; j < i; j++){
            follow_set follqcj = get_set(q)[cn][j];
            regexp * qj = (regexp*) follqcj.next;
            // We will test if: r|q = r|p /\ exists #: # |= psi1 /\ # |= psi2
            if(   qj->type == RE_ANY_CHAR
                || qi->type == RE_ANY_CHAR
                || qi->value.let_inf.let_val == qj->value.let_inf.let_val
            ){
              bool overlapping = true;
              for( con = 0; con < num_constraints; con++){
                bool this_overlapping = true;
                unsigned char updj, updi;
                updj = follqcj.updates[con];
                updi = follqci.updates[con];
                if( ( updi == FOLLOW_RES && updj == FOLLOW_INC ) || ( updi == FOLLOW_INC && updj == FOLLOW_RES ) ){
                  constraint c = re->constraints_r[con]->value.constr;
                  if ( c.lower == c.upper && c.upper != 0 ){
                    this_overlapping = false;
                  }
                }
                if(this_overlapping == false)
                  overlapping = false;
              }
              if(overlapping){
                unambiguous = false;
                break;
              }
              if(!unambiguous)
                break;
            }
            if(!unambiguous)
              break;
          }
          if(!unambiguous)
            break;
        }
        if(!unambiguous)
          break;
      }
      if(!unambiguous)
        break;
    }
  }
  return unambiguous;
}


void set_bin_firstlast(size_t * res_size, regexp *** res_set, unsigned int restype, bool nullable, size_t size1, size_t size2, regexp** set1, regexp** set2){
  *res_size = size1 + ( ((restype == RE_CONCAT && nullable) || restype == RE_CHOICE) ? size2 : 0);
  assert(*res_set == 0);
  *res_set = (regexp**) calloc(*res_size, sizeof(regexp*));
  memcpy( * res_set , set1, (sizeof(regexp*)) * size1);
  if((restype == RE_CONCAT && nullable) || restype == RE_CHOICE) {
    memcpy((* res_set) + size1, set2, (sizeof(regexp*)) * size2);
  }
}


void follow_setter_common(regexp * q, regexp * q1, regexp * re, follow_set ** follow, size_t * size_follow_set, size_t letterIndex){
  follow_set * foll_rq;
  assert(follow == q->follow_set || follow == q->prec_set);
  assert(size_follow_set == q->size_follow_set || size_follow_set == q->size_prec_set);
  follow[letterIndex] = (follow_set*) realloc(follow[letterIndex], sizeof(follow_set) * ( size_follow_set[letterIndex] + 1 ) );
  foll_rq = &(follow[letterIndex][size_follow_set[letterIndex]]);
  foll_rq->updates = (unsigned char *) calloc(updates_size, 1);
  foll_rq->next = q1;
}

/**
   Called by follow_set and prec_set in the case for concatenation
 **/
void follow_cat_setter(regexp* root, regexp * q, regexp * q1, regexp * re, follow_set ** follow, size_t * size_follow_set, size_t letterIndex){
  size_t c, con_no;
  follow_set * foll_rq;

  follow_setter_common(q, q1, re, follow, size_follow_set, letterIndex);
  foll_rq = &(follow[letterIndex][size_follow_set[letterIndex]]);
  for(c = 0; c < re->r1->num_constraints_r; c++){
    regexp* constr = re->r1->constraints_r[c];
    assert(constr->type == RE_CONSTR);
    if(position_incl(constr, q)){
      con_no = constr->value.constr.pos_constraints;
      assert(root->constraints_r[con_no] == constr);
      foll_rq->updates[con_no] = FOLLOW_RES;
    }
  }
  size_follow_set[letterIndex]++;
}

/**
   Called by follow_set and prec_set in the case for constraints
 **/
void follow_constr_setter(regexp* root, regexp * q, regexp * q1, regexp * re, follow_set ** follow, size_t * size_follow_set, size_t letterIndex){
  size_t c, con_no;
  follow_set * foll_rq;

  follow_setter_common(q, q1, re, follow, size_follow_set, letterIndex);
  foll_rq = &(follow[letterIndex][size_follow_set[letterIndex]]);
  for(c = 0; c < re->num_constraints_r; c++){
    regexp* constr = re->constraints_r[c];
    assert(constr->type == RE_CONSTR);
    con_no = constr->value.constr.pos_constraints;
    assert(root->constraints_r[con_no] == constr);
    if( re->constraints_r[c] == re){
      if(re->value.constr.lower != 0 || re->value.constr.upper != 0)
        foll_rq->updates[con_no] = FOLLOW_INC;
    } else if(position_incl(re->constraints_r[c], q)) {
      foll_rq->updates[con_no] = FOLLOW_RES;
    }
  }
  size_follow_set[letterIndex]++;
}

/**
   Sets correctly the members first_set and size_first_set in
   the whole tree of regexp under and including re
 **/
void set_first_last(regexp* re){
  assert( status_order == STATUS_POST_INIT );
  switch(re->type){
  case RE_EPSILON:
    re->size_first_set = re->size_last_set = 0;
    break;
  case RE_ANY_CHAR:
  case RE_CHAR_CLASS:
  case RE_LETTER:
    re->first_set = calloc(1, sizeof(regexp*));
    re->last_set = calloc(1, sizeof(regexp*));
    re->first_set[0] = re->last_set[0] = re;
    re->size_first_set = re->size_last_set = 1;
    break;
  case RE_CHOICE:
  case RE_CONCAT:
    set_first_last(re->r1);
    set_first_last(re->value.re2);
    set_bin_firstlast(
        & re->size_first_set,
        & re->first_set,
        re->type,
        re->r1->nullable,
        re->r1->size_first_set,
        re->value.re2->size_first_set,
        re->r1->first_set,
        re->value.re2->first_set
    );
    set_bin_firstlast(
        & re->size_last_set,
        & re->last_set,
        re->type,
        re->value.re2->nullable,
        re->value.re2->size_last_set,
        re->r1->size_last_set,
        re->value.re2->last_set,
        re->r1->last_set
    );
    break;
  case RE_CONSTR:
    set_first_last(re->r1);
    re->size_first_set = re->r1->size_first_set;
    re->first_set = re->r1->first_set;
    re->size_last_set = re->r1->size_last_set;
    re->last_set = re->r1->last_set;
    break;
  default:
    fprintf(stderr, "type %u has no defined first_set set\n", re->type);
    assert(false);
    break;
  }
  return;
}

/**
   Used for printing info about parsed regexp
 **/
void fprintf_space(FILE * f, size_t done){
  size_t i;
  for(i = done; i < 2 * max_pos + 11; i++)
    fprintf(f, " ");
  fprintf(f, " = ");
}

/**
   Returns true if
   the first position includes the second position,
   that is, if the subtree from the first include
   the subtree at the second position
 **/
bool position_incl(regexp *l, regexp* g){
  size_t i;
  if(l->tpos.nlist > g->tpos.nlist)
    return false;
  for(i = 0; i < l->tpos.nlist ; i++){
    if(l->tpos.list[i] != g->tpos.list[i]){
      return false;
    }
  }
  return true;
}

/**
   Returns true if position i is
   in the last set of re
 **/
bool in_last(regexp* re, size_t i){
  size_t j;
  for(j = 0; j < re->size_last_set; j++){
    if(re->last_set[j]->value.let_inf.pos_letters_pos == i){
      return true;
    }
  }
  return false;
}


/**
   Returns true if position i is
   in the first set of re
 **/
bool in_first(regexp* re, size_t i){
  size_t j;
  for(j = 0; j < re->size_first_set; j++){
    if(re->first_set[j]->value.let_inf.pos_letters_pos == i){
      return true;
    }
  }
  return false;
}

#include "follow_prec.c"

/**
   Prints the first set, a set of positions
 **/
void fprintf_first(FILE* f, regexp** first, size_t pos, size_t size){
  if(size > pos){
    fprintf(f, "<");
    fprintf_position(f, &(first[pos]->tpos));
    fprintf(f, ">");
    if(++pos < size) fprintf(f, ", ");
    fprintf_first(f, first, pos, size);
  }
  return;
}


/**
   Prints out a set of positions with name given
 **/
void fprintf_position_set(FILE* f, size_t num_positions, regexp** set_positions, char* name_set){
  size_t i;
  bool first_print = true;

  fprintf(f, "\n%s", name_set);
  fprintf_space(f, strlen(name_set));
  fprintf(f, "{");

  for(i = 0; i < num_positions; i++){
    if(!first_print)
      fprintf(f, ",");
    first_print = false;
    fprintf(f, "<");
    fprintf_position(f, &(set_positions[i]->tpos));
    fprintf(f, ">");
  }
  fprintf(f, "}");
}

/**
   Prints the F -- final set
 **/
void fprintf_final(FILE* f, regexp* re){
  size_t i, j;

  fprintf(f, "\nF(qI)");
  fprintf_space(f, 5);
  fprintf(f, (re->nullable) ? " {}":"false");

  for( i = 0; i < num_pos_letters; i++){
    bool first_print = true;
    regexp * cur_re = pos_letters[i];
    fprintf(f, "\nF(<");
    fprintf_position(f, &(cur_re->tpos));
    fprintf(f, ">)");
    fprintf_space(f, 4 + 2 * cur_re->tpos.nlist);

    if(!in_last(re, i)) {
      fprintf(f, "false");
    } else {
      for(j = 0; j < re->num_constraints_r; j++){
        if(position_incl(re->constraints_r[j], cur_re)){
          if(!first_print)
            fprintf(f,"false");
          first_print = false;
          fprintf(f,"<");
          fprintf_position(f, &(re->constraints_r[j]->tpos));
          fprintf(f,">");
        }
      }
      if(first_print)
        fprintf(f, "{}");
    }
  }
}

/**
   Prints out the delta function of the generated FAC
 **/

void fprintf_delta(FILE* f, regexp* re){
  size_t i;

  bool first_print = true;

  fprintf_position_set(f, num_pos_letters, pos_letters, "Q");

  assert(num_constraints == re->num_constraints_r);
  fprintf_position_set(f, num_constraints, re->constraints_r, "C");

  fprintf(f, "\n(min,max)");
  fprintf_space(f, 9);
  fprintf(f, "{");
  for(i = 0; i < num_constraints; i++){
    if(!first_print)
      fprintf(f, ",");
    first_print = false;
    fprintf(f, "<");
    fprintf_position(f, &(re->constraints_r[i]->tpos));
    fprintf(f, "> -> (%ui,%ui)",re->constraints_r[i]->value.constr.lower, re->constraints_r[i]->value.constr.upper);
  }
  fprintf(f, "}");

  fprintf(f, "}");
  for(i = 0; i < re->size_first_set; i++){
    fprintf(f, "\nphi(qI,%c)", re->first_set[i]->value.let_inf.let_val);
    fprintf_space(f, 7);
    fprintf(f, "(<");
    fprintf_position(f, &(re->first_set[i]->tpos));
    fprintf(f, ">,{})");
  }

  for(i = 0; i < num_pos_letters; i++){
    size_t c, p;
    regexp * cur_state = pos_letters[i];
    for(c = 0; c <= alph_size; c++){
      if(cur_state->size_follow_set[c] > 0) {
        fprintf(f, "\nphi(<");
        fprintf_position(f, &(cur_state->tpos));
        if(c == 0){
          fprintf(f, ">,any)");
          fprintf_space(f, 2 * cur_state->tpos.nlist + 8);
        }
        else {
          fprintf(f, ">,%c)", alphabet[c]);
          fprintf_space(f, 2 * cur_state->tpos.nlist + 6);
        }
        fprintf(f, "{");
        first_print = true;
        for(p = 0; p < cur_state->size_follow_set[c]; p ++){
          size_t u;
          bool first_update = true;
          follow_set * foll_s = &(cur_state->follow_set[c][p]);
          if(!first_print)
            fprintf(f, ",");
          first_print = false;
          fprintf(f, "(<");
          fprintf_position(f,  & (( (regexp*) (foll_s->next))->tpos) );
          fprintf(f, ">,{");
          for(u = 0; u < num_constraints; u++){
            if(foll_s->updates[u] != FOLLOW_NIL){
              assert(foll_s->updates[u] == FOLLOW_RES ||  foll_s->updates[u] == FOLLOW_INC);
              if(!first_update)
                fprintf(f, ",");
              first_update = false;
              fprintf(f,"<");
              fprintf_position(f, &(re->constraints_r[u]->tpos));
              fprintf(f, "> -> %s", (foll_s->updates[u] == FOLLOW_RES) ? "res" : "inc");
            }
          }
          fprintf(f, "})");
          if(p+1 < re->size_follow_set[c])
            fprintf(f, ",");
        }
        fprintf(f,"}");
      }
    }
  }
  fprintf_final(f, re);
  fprintf(f,"\n");

}
/**
    Prints out a position
 **/
void fprintf_position(FILE* f, re_pos* pos){
  size_t i;
  if(pos->nlist > 0){
    for(i = 0; i < pos->nlist - 1; i++){
      fprintf(f, "%u,", (unsigned int) pos->list[i]);
    }
    fprintf(f, "%u", (unsigned int) pos->list[pos->nlist-1]);
  }
  return;
}


void fprintf_follow_sets(FILE* f, regexp* re){
  size_t i;
  for(i = 0; i < num_pos_letters; i++){
    fprintf_follow_set(f, pos_letters[i], re->constraints_r);
  }
  fprintf(f,"\n");
}


void fprintf_follow_set(FILE* f, regexp* re, regexp** constraints){
  size_t c, p;
  for(c = 0; c <= alph_size; c++){
    if(re->size_follow_set[c] > 0) {
      fprintf(f, "\nphi(<");
      fprintf_position(f, &(re->tpos));
      fprintf(f, ">,");
      if(c == 0)
        fprintf(f, "any");
      else
        fprintf(f, "%c", alphabet[c]);
      fprintf(f, ")");
      fprintf_space(f, 2 * re->tpos.nlist + 11);
      fprintf(f, "{");
      for(p = 0; p < re->size_follow_set[c]; p ++){
        size_t u;
        bool first_update = true;
        follow_set * foll_s = &(re->follow_set[c][p]);
        fprintf(f, "(<");
        fprintf_position(f,  & (( (regexp*) (foll_s->next))->tpos) );
        fprintf(f, ">,{");
        for(u = 0; u < num_constraints; u++){
          if(foll_s->updates[u] != FOLLOW_NIL){
            assert(foll_s->updates[u] == FOLLOW_RES ||  foll_s->updates[u] == FOLLOW_INC);
            if(!first_update)
              fprintf(f, ",");
            first_update = false;
            fprintf(f,"<");
            fprintf_position(f, &(constraints[u]->tpos));
            fprintf(f, "> -> %s", (foll_s->updates[u] == FOLLOW_RES) ? "res" : "inc");
          }
        }
        fprintf(f, "})");
        if(p+1 < re->size_follow_set[c])
          fprintf(f, ",");
      }
      fprintf(f,"}");
    }
  }
}


void fprintf_alphabet(FILE * f){
  size_t i;
  fprintf(f, "{");
  for(i = 1; i <= alph_size; i++){
    fprintf(f, "%c", alphabet[i]);
    if(i < alph_size) fprintf(f, ", ");
  }
  fprintf(f, "}");
}

void fprint_regexp(FILE* f, regexp* re){
  switch(re->type){
  case RE_EPSILON:
    fprintf(f, "epsilon");
    break;
  case RE_LETTER :
    fprintf(f, "%c", re->value.let_inf.let_val);
    break;
  case RE_ANY_CHAR :
    fprintf(f, ".");
    break;
  case RE_CHOICE:
    fprintf(f,"(");
    fprint_regexp(f, re->r1);
    fprintf(f,"|");
    fprint_regexp(f, re->value.re2);
    fprintf(f,")");
    break;
  case RE_CONCAT:
    fprintf(f,"(");
    fprint_regexp(f, re->r1);
    fprintf(f,"*");
    fprint_regexp(f, re->value.re2);
    fprintf(f,")");
    break;
  case RE_CONSTR:
    fprintf(f,"(");
    fprint_regexp(f, re->r1);
    fprintf(f,"{%u,%u})", re->value.constr.lower, re->value.constr.upper);
    break;
  case RE_CHAR_CLASS:
    fprintf(f,"[%c-%c]", re->value.let_inf.let_val, re->value.let_inf.upper);
    break;
  default:
    fprintf(stderr,"Error in expression: Unknown type %u", re->type);
    assert(false);
  }
}


/**
   Prints out the term tree of  mu(r), the tree where letters are replaced with positions
 **/
void fprint_mu_re(FILE* f, regexp* re){
  switch(re->type){
  case RE_EPSILON:
  case RE_CHAR_CLASS:
  case RE_LETTER :
    fprintf(f, "<");
    fprintf_position(f, &(re->tpos));
    fprintf(f, ">");
    break;
  case RE_CHOICE:
    fprintf(f,"(");
    fprint_mu_re(f, re->r1);
    fprintf(f,"|");
    fprint_mu_re(f, re->value.re2);
    fprintf(f,")");
    break;
  case RE_CONCAT:
    fprintf(f,"(");
    fprint_mu_re(f, re->r1);
    fprintf(f,"*");
    fprint_mu_re(f, re->value.re2);
    fprintf(f,")");
    break;
  case RE_CONSTR:
    fprintf(f,"(");
    fprint_mu_re(f, re->r1);
    fprintf(f,"{%u,%u})", re->value.constr.lower, re->value.constr.upper);
    break;
  default:
    fprintf(stderr,"Error in expression: unknown type: %u\n", re->type);
    assert(false);
  }
}


void free_mem(){
  //if(mem_buf != 0)
  //  free(mem_buf);
  free(re_buf);
  free(charAlphPos);
  free(alphabet);
}

