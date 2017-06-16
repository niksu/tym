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
  char h = hash_term(term);

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
      enum eq_term_error error_code;
      if (eq_term(term, cursor->term, &error_code, &result)) {
          exists = result;
          break;
      } else {
        printf("Error when comparing terms for equality: %d", error_code);
        assert(false);
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

struct buffer_write_result *
term_database_str(struct term_database_t * tdb, struct buffer_info * dst)
{
  assert(NULL != tdb);
  assert(NULL != dst);

  size_t initial_idx = dst->idx;

  const struct terms_t * cursor = tdb->herbrand_universe;

  struct buffer_write_result * res = NULL;

  while (NULL != cursor) {
    res = term_to_str(cursor->term, dst);
    assert(is_ok_buffer_write_result(res));
    free(res);

    safe_buffer_replace_last(dst, '\n');

    cursor = cursor->next;
  }

  return mkval_buffer_write_result(dst->idx - initial_idx);
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

void
free_pred(struct predicate_t * pred)
{
  // NOTE we assume that the data in the pred's fields are shared,
  //      so we don't free that memory.
  free(pred);
}

DEFINE_MUTABLE_LIST_MK(predicate, pred, struct predicate_t, struct predicates_t)

bool
eq_pred(struct predicate_t p1, struct predicate_t p2, enum eq_pred_error * error_code, bool * result)
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
atom_database_member(const struct atom_t * atom, struct atom_database_t * adb, enum adl_lookup_error * error_code, struct predicate_t ** record)
{
  bool success;
  if (NULL == adb) {
    success = true;
    *record = NULL;
  } else {
    char h = hash_str(atom->predicate);

    if (NULL == adb->atom_database[(int)h]) {
      success = true;
      *record = NULL;
    } else {
      bool exists = false;

      struct predicate_t * pred = mk_pred(atom->predicate, atom->arity);

      struct predicates_t * cursor = adb->atom_database[(int)h];

      do {
        enum eq_pred_error eq_pred_error_code;
        success = (eq_pred(*pred, *(cursor->predicate), &eq_pred_error_code, &exists));
        if (success) {
          if (exists) {
            *record = cursor->predicate;
            free_pred(pred);
            return true;
          }
        } else {
          assert(SAME_PREDICATE_DIFF_ARITY == eq_pred_error_code);
          *error_code = DIFF_ARITY;
        }

        cursor = cursor->next;
      } while (success && NULL != cursor);

      free_pred(pred);
    }
  }

  *record = NULL;
  return success;
}

bool
atom_database_add(const struct atom_t * atom, struct atom_database_t * adb, enum adl_add_error * error_code, struct predicate_t ** result)
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
      // NOTE we don't need to check return value here, since it simply
      //      indicates whether ther term already existed or not in the term
      //      database.
      (void)term_database_add(atom->args[i], adb->tdb);
    }
  }

  return success;
}

struct buffer_write_result *
atom_database_str(struct atom_database_t * adb, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = buf_strcpy(dst, "Terms:");
  assert(is_ok_buffer_write_result(res));
  free(res);

  safe_buffer_replace_last(dst, '\n');

  res = term_database_str(adb->tdb, dst);
  assert(is_ok_buffer_write_result(res));
  free(res);

  safe_buffer_replace_last(dst, '\n');

  res = buf_strcpy(dst, "Predicates:");
  assert(is_ok_buffer_write_result(res));
  free(res);

  safe_buffer_replace_last(dst, '\n');

  const struct predicates_t * cursor;

  for (int i = 0; i < ATOM_DATABASE_SIZE; i++) {
    cursor = adb->atom_database[i];
    while (NULL != cursor) {
      res = predicate_str(cursor->predicate, dst);
      assert(is_ok_buffer_write_result(res));
      free(res);

      safe_buffer_replace_last(dst, '\n');

      const struct clauses_t * clause_cursor = cursor->predicate->bodies;

      while (NULL != clause_cursor) {

        if (have_space(dst, 1)) {
          unsafe_buffer_str(dst, "  *");
          safe_buffer_replace_last(dst, ' ');
        } else {
          return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
        }

        res = clause_to_str(clause_cursor->clause, dst);
        assert(is_ok_buffer_write_result(res));
        free(res);

        safe_buffer_replace_last(dst, '\n');

        clause_cursor = clause_cursor->next;
      }

      cursor = cursor->next;
    }
  }

  return mkval_buffer_write_result(dst->idx - initial_idx);
}

struct buffer_write_result *
predicate_str(const struct predicate_t * pred, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = buf_strcpy(dst, pred->predicate);
  assert(is_ok_buffer_write_result(res));
  free(res);

  safe_buffer_replace_last(dst, '/');

  char buf[BUF_SIZE];
  int check = sprintf(buf, "%u", pred->arity);
  assert(check > 0);

  res = buf_strcpy(dst, buf);
  assert(is_ok_buffer_write_result(res));
  free(res);

  return mkval_buffer_write_result(dst->idx - initial_idx);
}

struct predicates_t *
atom_database_to_predicates(struct atom_database_t * adb)
{
  struct predicates_t * result = NULL;
  struct predicates_t * result_cursor = NULL;
  const struct predicates_t * cursor = NULL;

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
clause_database_add(struct clause_t * clause, struct atom_database_t * adb, enum cdl_add_error * cdl_add_error)
{
  enum adl_lookup_error adl_lookup_error;
  enum adl_add_error adl_add_error;
  struct predicate_t * record = NULL;
  bool success = atom_database_member(clause->head, adb, &adl_lookup_error, &record);
  if (!success) {
    assert(DIFF_ARITY == adl_lookup_error);
    *cdl_add_error = CDL_ADL_DIFF_ARITY;
    ERR("Looking-up atom failed (%d, %d): %s\n", adl_lookup_error,
         *cdl_add_error, clause->head->predicate);
    return false;
  } else if (NULL == record) {
    struct predicate_t * result;
    success = atom_database_add(clause->head, adb, &adl_add_error, &result);
    result->bodies = mk_clause_cell(clause, NULL);
    if (!success) {
      assert(NO_ATOM_DATABASE == adl_add_error);
      *cdl_add_error = CDL_ADL_NO_ATOM_DATABASE;
      ERR("Adding atom failed (%d, %d): %s\n", adl_add_error,
           *cdl_add_error, clause->head->predicate);
      return false;
    }
  } else if (success && NULL != record) {
    assert(NULL != adb->tdb);
    for (int i = 0; i < clause->head->arity; i++) {
      // NOTE we don't need to check return value here, since it simply
      //      indicates whether ther term already existed or not in the term
      //      database.
      (void)term_database_add(clause->head->args[i], adb->tdb);
    }

    struct clauses_t * remainder = record->bodies;
    record->bodies = mk_clause_cell(clause, remainder);
  }

  if (success) {
    for (int i = 0; success && i < clause->body_size; i++) {
      success &= atom_database_member(clause->body[i], adb, &adl_lookup_error, &record);
      if (!success) {
        assert(DIFF_ARITY == adl_lookup_error);
        *cdl_add_error = CDL_ADL_DIFF_ARITY;
        ERR("Looking-up atom failed (%d, %d): %s\n", adl_lookup_error,
             *cdl_add_error, clause->body[i]->predicate);
        return false;
      } else if (NULL == record) {
        struct predicate_t * result;
        success &= atom_database_add(clause->body[i], adb, &adl_add_error, &result);
        if (!success) {
          assert(NO_ATOM_DATABASE == adl_add_error);
          *cdl_add_error = CDL_ADL_NO_ATOM_DATABASE;
          ERR("Adding atom failed (%d, %d): %s\n", adl_add_error,
               *cdl_add_error, clause->body[i]->predicate);
          return false;
        }
      }
    }
  }

  return success;
}

size_t
num_predicate_bodies (struct predicate_t * p)
{
  size_t no_bodies = 0;
  const struct clauses_t * body_cursor = p->bodies;
  while (NULL != body_cursor) {
    no_bodies++;
    body_cursor = body_cursor->next;
  }
  return no_bodies;
}
