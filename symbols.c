/*
 * Symbol table.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include "symbols.h"

struct term_database_t *
mk_term_database(void)
{
  struct term_database_t * result = malloc(sizeof(struct term_database_t));
  result ->herbrand_universe = NULL;
  for (int i = 0; i < TERM_DATABASE_SIZE; i++) {
    result->term_database[i] = NULL;
  }
  return result;
}

bool
term_database_add(struct term_t * term, struct term_database_t * tdb)
{
  bool exists = false;
  char h = hash_term(*term);

  DBG("Trying adding to Herbrand universe: %s\n", term->identifier);

  if (CONST != term->kind) {
    return false;
  }

  if (NULL == tdb->term_database[(int)h]) {
    tdb->term_database[(int)h] = mk_term_cell(term, NULL);
    tdb->herbrand_universe = mk_term_cell(term, tdb->herbrand_universe);
    DBG("Added to Herbrand universe: %s\n", term->identifier);
  } else {
    struct terms_t * cursor = tdb->term_database[(int)h];
    do {
      bool result;
      eq_term_error_t error_code;
      if (eq_term(*term, *(cursor->term), &error_code, &result)) {
          exists = result;
          break;
      } else {
        // FIXME analyse and and act on error_code.
      }
    } while (NULL != cursor->next);

    if (!exists) {
      cursor->next = mk_term_cell(term, NULL);
      tdb->herbrand_universe = mk_term_cell(term, tdb->herbrand_universe);
      DBG("Added to Herbrand universe: %s\n", term->identifier);
    }
  }

  return exists;
}

int
term_database_str(struct term_database_t * tdb, size_t * outbuf_size, char * outbuf)
{
  assert(NULL != tdb);
  assert(NULL != outbuf);

  struct terms_t * cursor = tdb->herbrand_universe;
  int l = 0;

  while (NULL != cursor && *outbuf_size > 0) {
    int l_sub = term_to_str(cursor->term, outbuf_size, outbuf + l);
    if (l_sub < 0) {
      // FIXME complain
      return l_sub;
    }

    l += l_sub;

    outbuf[(*outbuf_size)--, l++] = '\n';
    assert(*outbuf_size > 0);

    cursor = cursor->next;
  }

  return l;
}

struct predicate_t *
mk_pred(const char * predicate, uint8_t arity)
{
  assert(NULL != predicate);

  struct predicate_t * p = malloc(sizeof(struct predicate_t));
  assert(NULL != p);

  p->predicate = predicate;
  p->arity = arity;
  p->bodies = NULL;
  return p;
}

struct predicates_t *
mk_pred_cell(struct predicate_t * pred, struct predicates_t * next)
{
  assert(NULL != pred);

  struct predicates_t * ps = malloc(sizeof(struct predicates_t));
  assert(NULL != ps);

  ps->predicate = pred;
  ps->next = next;
  return ps;
}

bool
eq_pred(struct predicate_t p1, struct predicate_t p2, eq_pred_error_t * error_code, bool * result)
{
  *error_code = NO_ERROR_eq_pred;

  if (&p1 == &p2) {
    *result = true;
    return true;
  }

  if (0 != strcmp(p1.predicate, p2.predicate)) {
    *result = false;
    return true;
  } else {
    if (p1.arity == p2.arity) {
      *result = true;
      return true;
    } else {
      *error_code = SAME_PREDICATE_DIFF_ARITY;
      return false;
    }
  }
}

struct atom_database_t *
mk_atom_database(void)
{
  struct atom_database_t * result = malloc(sizeof(struct atom_database_t));
  result->tdb = mk_term_database();
  for (int i = 0; i < ATOM_DATABASE_SIZE; i++) {
    result->atom_database[i] = NULL;
  }
  return result;
}

bool
atom_database_member(struct atom_t * atom, struct atom_database_t * adb, adl_lookup_error_t * error_code, struct predicate_t ** record)
{
  bool success;
  if (NULL == adb) {
    success = true;
    *record = NULL;
  } else {
    char h = hash_str(atom->predicate);

    struct predicate_t * pred = mk_pred(atom->predicate, atom->arity);

    if (NULL == adb->atom_database[(int)h]) {
      success = true;
      *record = NULL;
    } else {
      bool exists = false;

      struct predicates_t * cursor = adb->atom_database[(int)h];
      do {
        eq_pred_error_t eq_pred_error_code;
        success = (eq_pred(*pred, *(cursor->predicate), &eq_pred_error_code, &exists));
        if (success) {
          if (exists) {
            *record = cursor->predicate;
            return true;
          }
        } else {
          // FIXME analyse and and act on error_code.
        }

        cursor = cursor->next;
      } while (success && NULL != cursor);
    }
  }

  *record = NULL;
  return success;
}

bool
atom_database_add(struct atom_t * atom, struct atom_database_t * adb, adl_add_error_t * error_code, struct predicate_t ** result)
{
  bool success;

  if (NULL == adb) {
    *error_code = NO_ATOM_DATABASE;
    success = false;
  } else {
    char h = hash_str(atom->predicate);

    struct predicate_t * pred = mk_pred(atom->predicate, atom->arity);

    if (NULL == adb->atom_database[(int)h]) {
      adb->atom_database[(int)h] = mk_pred_cell(pred, NULL);
    } else {
      adb->atom_database[(int)h] = mk_pred_cell(pred, adb->atom_database[(int)h]);
    }

    *result = pred;
    success = true;

    DBG("Added atom: %s{hash=%u}\n", atom->predicate, h);

    assert(NULL != adb->tdb);
    for (int i = 0; i < atom->arity; i++) {
      // FIXME check return value
      (void)term_database_add(&(atom->args[i]), adb->tdb);
    }
  }

  return success;
}

int
atom_database_str(struct atom_database_t * adb, size_t * outbuf_size, char * outbuf)
{
  int l = my_strcpy(outbuf, "Terms:\n", outbuf_size);
  if (l < 0) {
    // FIXME complain
  }

  int l_sub = term_database_str(adb->tdb, outbuf_size, outbuf + l);
  if (l_sub < 0) {
    // FIXME complain
  }
  l += l_sub;
  outbuf[(*outbuf_size)--, l++] = '\n';
  l_sub = my_strcpy(&(outbuf[l]), "Predicates:\n", outbuf_size);
  if (l_sub < 0) {
    // FIXME complain
  }
  l += l_sub;

  // FIXME any time we detect that *outbuf_size > 0 then
  //       switch to failure mode.

  struct predicates_t * cursor;

  for (int i = 0; i < ATOM_DATABASE_SIZE; i++) {
    cursor = adb->atom_database[i];
    while (NULL != cursor && *outbuf_size > 0) {
      l_sub = predicate_str(cursor->predicate, outbuf_size, outbuf + l);
      if (l_sub < 0) {
        // FIXME complain
      }
      l += l_sub;

      outbuf[(*outbuf_size)--, l++] = '\n';

      struct clauses_t * clause_cursor = cursor->predicate->bodies;
      while (NULL != clause_cursor) {
        outbuf[(*outbuf_size)--, l++] = ' ';
        outbuf[(*outbuf_size)--, l++] = ' ';
        outbuf[(*outbuf_size)--, l++] = '*';
        outbuf[(*outbuf_size)--, l++] = ' ';
        l_sub = clause_to_str(clause_cursor->clause, outbuf_size, outbuf + l);
        if (l_sub < 0) {
          // FIXME complain
        }
        l += l_sub;
        outbuf[(*outbuf_size)--, l++] = '\n';

        clause_cursor = clause_cursor->next;
      }

      cursor = cursor->next;
    }
  }

  assert(*outbuf_size > 0);

  return l;
}

int
predicate_str(struct predicate_t * pred, size_t * outbuf_size, char * outbuf)
{
  int l = 0;
  int l_sub = my_strcpy(&(outbuf[l]), pred->predicate, outbuf_size);
  if (l_sub < 0) {
    // FIXME complain
  }
  l += l_sub;

  outbuf[(*outbuf_size)--, l++] = '/';

  l_sub = sprintf(&(outbuf[l]), "%u", pred->arity);
  if (l_sub < 0) {
    // FIXME complain
  }
  *outbuf_size -= strlen(&(outbuf[l]));
  l += l_sub;

  assert(*outbuf_size > 0);

  return l;
}

struct predicates_t *
atom_database_to_predicates(struct atom_database_t * adb)
{
  struct predicates_t * result = NULL;
  struct predicates_t * result_cursor;
  struct predicates_t * cursor;

  for (int i = 0; i < ATOM_DATABASE_SIZE; i++) {
    cursor = adb->atom_database[i];
    while (NULL != cursor) {
      if (NULL == result) {
        assert(NULL == result_cursor);
        result_cursor = malloc(sizeof(*result_cursor));
        result = result_cursor;
      } else {
        assert(NULL != result_cursor);
        result_cursor->next = malloc(sizeof(*result_cursor));
        result_cursor = result_cursor->next;
      }

      result_cursor->predicate = cursor->predicate;
      result_cursor->next = NULL;

      cursor = cursor->next;
    }
  }

  return result;
}

bool
clause_database_add(struct clause_t * clause, struct atom_database_t * adb, void * cdl_add_error)
{
  adl_lookup_error_t adl_lookup_error;
  adl_add_error_t adl_add_error;
  struct predicate_t * record;
  bool success = atom_database_member(&clause->head, adb, &adl_lookup_error, &record);
  if (!success) {
    // FIXME elaborate this further
    ERR("Looking-up atom failed: %s\n", clause->head.predicate);
    return false; // FIXME set 'cdl_add_error'
  } else if (NULL == record) {
    struct predicate_t * result;
    success = atom_database_add(&clause->head, adb, &adl_add_error, &result);
    result->bodies = mk_clause_cell(clause, NULL);
    if (!success) {
      // FIXME elaborate this further
      ERR("Adding atom failed: %s\n", clause->head.predicate);
    }
  } else if (success && NULL != record) {
    assert(NULL != adb->tdb);
    for (int i = 0; i < clause->head.arity; i++) {
      // FIXME check return value
      (void)term_database_add(&(clause->head.args[i]), adb->tdb);
    }

    struct clauses_t * remainder = record->bodies;
    record->bodies = mk_clause_cell(clause, remainder);
  }

  // FIXME check adl_add_error
  if (success) {
    for (int i = 0; success && i < clause->body_size; i++) {
      success &= atom_database_member(&clause->body[i], adb, &adl_lookup_error, &record);
      if (!success) {
        // FIXME elaborate this further
        ERR("Looking-up atom failed: %s\n", clause->body[i].predicate);
      } else if (NULL == record) {
        struct predicate_t * result;
        success &= atom_database_add(&clause->body[i], adb, &adl_add_error, &result);
        if (!success) {
          // FIXME elaborate this further
          ERR("Adding atom failed: %s\n", clause->body[i].predicate);
        }
      }
    }
  }

  return success;
}
