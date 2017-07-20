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
  struct term_database_t * result = malloc(sizeof *result);
  result ->herbrand_universe = NULL;
  for (int i = 0; i < TERM_DATABASE_SIZE; i++) {
    result->term_database[i] = NULL;
  }
  return result;
}

bool
term_database_add(struct TymTerm * term, struct term_database_t * tdb)
{
  bool exists = false;
  char h = tym_hash_term(term);

  TYM_DBG("Trying adding to Herbrand universe: %s\n", term->identifier);

  if (TYM_CONST != term->kind) {
    return false;
  }

  if (NULL == tdb->term_database[(int)h]) {
    tdb->term_database[(int)h] = tym_mk_term_cell(tym_copy_term(term), NULL);
    tdb->herbrand_universe = tym_mk_term_cell(tym_copy_term(term), tdb->herbrand_universe);
    TYM_DBG("Added to Herbrand universe: %s\n", term->identifier);
  } else {
    struct TymTerms * cursor = tdb->term_database[(int)h];
    do {
      bool result;
      enum TymEqTermError error_code;
      if (tym_eq_term(term, cursor->term, &error_code, &result)) {
          exists = result;
          break;
      } else {
        printf("Error when comparing terms for equality: %d", error_code);
        assert(false);
      }
    } while (NULL != cursor->next);

    if (!exists) {
      cursor->next = tym_mk_term_cell(tym_copy_term(term), NULL);
      tdb->herbrand_universe = tym_mk_term_cell(tym_copy_term(term), tdb->herbrand_universe);
      TYM_DBG("Added to Herbrand universe: %s\n", term->identifier);
    }
  }

  return exists;
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
term_database_str(struct term_database_t * tdb, struct TymBufferInfo * dst)
{
  assert(NULL != tdb);
  assert(NULL != dst);

  size_t initial_idx = dst->idx;

  const struct TymTerms * cursor = tdb->herbrand_universe;

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  while (NULL != cursor) {
    res = tym_term_to_str(cursor->term, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_safe_buffer_replace_last(dst, '\n');

    cursor = cursor->next;
  }

  return tym_mkval_TymBufferWriteResult(dst->idx - initial_idx);
}

struct predicate_t *
mk_pred(const char * predicate, uint8_t arity)
{
  assert(NULL != predicate);

  struct predicate_t * p = malloc(sizeof *p);
  assert(NULL != p);

  p->predicate = predicate;
  p->arity = arity;
  p->bodies = NULL;
  return p;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_pred(struct predicate_t * pred)
{
  free((void *)pred->predicate);
  if (NULL != pred->bodies) {
    tym_free_clauses(pred->bodies);
  }
  free(pred);
}
#pragma GCC diagnostic pop

TYM_DEFINE_MUTABLE_LIST_MK(predicate, pred, struct predicate_t, struct predicates_t)

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
  struct atom_database_t * result = malloc(sizeof *result);
  result->tdb = mk_term_database();
  for (int i = 0; i < ATOM_DATABASE_SIZE; i++) {
    result->atom_database[i] = NULL;
  }
  return result;
}

bool
atom_database_member(const struct TymAtom * atom, struct atom_database_t * adb, enum adl_lookup_error * error_code, struct predicate_t ** record)
{
  bool success;
  if (NULL == adb) {
    success = true;
    *record = NULL;
  } else {
    char h = tym_hash_str(atom->predicate);

    if (NULL == adb->atom_database[(int)h]) {
      success = true;
      *record = NULL;
    } else {
      bool exists = false;

      struct predicate_t * pred = mk_pred(strdup(atom->predicate), atom->arity);

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
    }
  }

  *record = NULL;
  return success;
}

bool
atom_database_add(const struct TymAtom * atom, struct atom_database_t * adb, enum adl_add_error * error_code, struct predicate_t ** result)
{
  bool success;

  if (NULL == adb) {
    *error_code = NO_ATOM_DATABASE;
    success = false;
  } else {
    char h = tym_hash_str(atom->predicate);

    struct predicate_t * pred = mk_pred(strdup(atom->predicate), atom->arity);

    if (NULL == adb->atom_database[(int)h]) {
      adb->atom_database[(int)h] = tym_mk_pred_cell(pred, NULL);
    } else {
      bool exists = false;
      struct predicates_t * cursor = adb->atom_database[(int)h];
      while (NULL != cursor) {
        enum eq_pred_error eq_pred_error_code;
        bool eq_pred_result;
        if (eq_pred(*pred, *cursor->predicate, &eq_pred_error_code, &eq_pred_result)) {
          free_pred(pred);
          pred = cursor->predicate;
          exists = true;
        } else {
          printf("Error when comparing terms for equality: %d", eq_pred_error_code);
          assert(false);
        }
        cursor = cursor->next;
      }
      if (!exists) {
        adb->atom_database[(int)h] = tym_mk_pred_cell(pred, adb->atom_database[(int)h]);
      }
    }

    *result = pred;
    success = true;

    TYM_DBG("Added atom: %s{hash=%u}\n", atom->predicate, h);

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

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
atom_database_str(struct atom_database_t * adb, struct TymBufferInfo * dst)
{
  assert(NULL != adb);

  size_t initial_idx = dst->idx;

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_buf_strcpy(dst, "Terms:");
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_safe_buffer_replace_last(dst, '\n');

  res = term_database_str(adb->tdb, dst);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_safe_buffer_replace_last(dst, '\n');

  res = tym_buf_strcpy(dst, "Predicates:");
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_safe_buffer_replace_last(dst, '\n');

  const struct predicates_t * cursor;

  for (int i = 0; i < ATOM_DATABASE_SIZE; i++) {
    cursor = adb->atom_database[i];
    while (NULL != cursor) {
      res = predicate_str(cursor->predicate, dst);
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      tym_safe_buffer_replace_last(dst, '\n');

      const struct TymClauses * clause_cursor = cursor->predicate->bodies;

      while (NULL != clause_cursor) {

        if (tym_have_space(dst, 1)) {
          tym_unsafe_buffer_str(dst, "  *");
          tym_safe_buffer_replace_last(dst, ' ');
        } else {
          return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
        }

        res = tym_clause_to_str(clause_cursor->clause, dst);
        assert(tym_is_ok_TymBufferWriteResult(res));
        free(res);

        tym_safe_buffer_replace_last(dst, '\n');

        clause_cursor = clause_cursor->next;
      }

      cursor = cursor->next;
    }
  }

  return tym_mkval_TymBufferWriteResult(dst->idx - initial_idx);
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
predicate_str(const struct predicate_t * pred, struct TymBufferInfo * dst)
{
  size_t initial_idx = dst->idx;

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_buf_strcpy(dst, pred->predicate);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_safe_buffer_replace_last(dst, '/');

  char buf[TYM_BUF_SIZE];
  int check = sprintf(buf, "%u", pred->arity);
  assert(check > 0);

  res = tym_buf_strcpy(dst, buf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  return tym_mkval_TymBufferWriteResult(dst->idx - initial_idx);
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
        result_cursor = malloc(sizeof *result_cursor);
        result = result_cursor;
      } else {
        assert(NULL != result_cursor);
        result_cursor->next = malloc(sizeof *result_cursor);
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
clause_database_add(struct TymClause * clause, struct atom_database_t * adb, enum cdl_add_error * cdl_add_error)
{
  enum adl_lookup_error adl_lookup_error;
  enum adl_add_error adl_add_error;
  struct predicate_t * record = NULL;
  bool success = atom_database_member(clause->head, adb, &adl_lookup_error, &record);
  if (!success) {
    assert(DIFF_ARITY == adl_lookup_error);
    *cdl_add_error = CDL_ADL_DIFF_ARITY;
    TYM_ERR("Looking-up atom failed (%d, %d): %s\n", adl_lookup_error,
         *cdl_add_error, clause->head->predicate);
    return false;
  } else if (NULL == record) {
    struct predicate_t * result;
    success = atom_database_add(clause->head, adb, &adl_add_error, &result);
    result->bodies = tym_mk_clause_cell(tym_copy_clause(clause), NULL);
    if (!success) {
      assert(NO_ATOM_DATABASE == adl_add_error);
      *cdl_add_error = CDL_ADL_NO_ATOM_DATABASE;
      TYM_ERR("Adding atom failed (%d, %d): %s\n", adl_add_error,
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

    struct TymClauses * remainder = record->bodies;
    record->bodies = tym_mk_clause_cell(tym_copy_clause(clause), remainder);
  }

  if (success) {
    for (int i = 0; success && i < clause->body_size; i++) {
      success &= atom_database_member(clause->body[i], adb, &adl_lookup_error, &record);
      if (!success) {
        assert(DIFF_ARITY == adl_lookup_error);
        *cdl_add_error = CDL_ADL_DIFF_ARITY;
        TYM_ERR("Looking-up atom failed (%d, %d): %s\n", adl_lookup_error,
             *cdl_add_error, clause->body[i]->predicate);
        return false;
      } else if (NULL == record) {
        struct predicate_t * result;
        success &= atom_database_add(clause->body[i], adb, &adl_add_error, &result);
        if (!success) {
          assert(NO_ATOM_DATABASE == adl_add_error);
          *cdl_add_error = CDL_ADL_NO_ATOM_DATABASE;
          TYM_ERR("Adding atom failed (%d, %d): %s\n", adl_add_error,
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
  const struct TymClauses * body_cursor = p->bodies;
  while (NULL != body_cursor) {
    no_bodies++;
    body_cursor = body_cursor->next;
  }
  return no_bodies;
}

void
free_atom_database(struct atom_database_t * adb)
{
  for (int i = 0; i < TERM_DATABASE_SIZE; i++) {
    struct TymTerms * cursor = adb->tdb->term_database[i];
    while (NULL != cursor) {
      struct TymTerms * pre_cursor = cursor;
      cursor = cursor->next;
      tym_free_term(pre_cursor->term);
      free(pre_cursor);
    }
  }
  {
    struct TymTerms * cursor = adb->tdb->herbrand_universe;
    while (NULL != cursor) {
      struct TymTerms * pre_cursor = cursor;
      cursor = cursor->next;
      tym_free_term(pre_cursor->term);
      free(pre_cursor);
    }
  }
  free(adb->tdb);

  for (int i = 0; i < ATOM_DATABASE_SIZE; i++) {
    struct predicates_t * cursor = adb->atom_database[i];
    while (NULL != cursor) {
      struct predicates_t * pre_cursor = cursor;
      cursor = cursor->next;
      free_pred(pre_cursor->predicate);
      free(pre_cursor);
    }
  }

  free(adb);
}
