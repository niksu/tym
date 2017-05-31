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
      if (eq_term(term, cursor->term, &error_code, &result)) {
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

size_t
term_database_str(struct term_database_t * tdb, size_t * outbuf_size, char * outbuf)
{
  assert(NULL != tdb);
  assert(NULL != outbuf);

  const struct terms_t * cursor = tdb->herbrand_universe;
  size_t l = 0;

  while (NULL != cursor && *outbuf_size > 0) {
    size_t l_sub = term_to_str(cursor->term, outbuf_size, outbuf + l);
    // FIXME check and react to errors.
    l += l_sub;

    outbuf[(*outbuf_size)--, l++] = '\n';
    assert(*outbuf_size > 0);

    cursor = cursor->next;
  }

  return l;
}

struct buffer_write_result *
Bterm_database_str(struct term_database_t * tdb, struct buffer_info * dst)
{
  assert(NULL != tdb);
  assert(NULL != dst);

  size_t initial_idx = dst->idx;

  const struct terms_t * cursor = tdb->herbrand_universe;

  struct buffer_write_result * res = NULL;

  while (NULL != cursor) {
    res = Bterm_to_str(cursor->term, dst);
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
atom_database_member(const struct atom_t * atom, struct atom_database_t * adb, adl_lookup_error_t * error_code, struct predicate_t ** record)
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
        eq_pred_error_t eq_pred_error_code;
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
atom_database_add(const struct atom_t * atom, struct atom_database_t * adb, adl_add_error_t * error_code, struct predicate_t ** result)
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

size_t
atom_database_str(struct atom_database_t * adb, size_t * outbuf_size, char * outbuf)
{
  size_t l = my_strcpy(outbuf, "Terms:\n", outbuf_size);
  // FIXME check and react to errors.

  size_t l_sub = term_database_str(adb->tdb, outbuf_size, outbuf + l);
  // FIXME check and react to errors.
  l += l_sub;
  outbuf[(*outbuf_size)--, l++] = '\n';
  l_sub = my_strcpy(&(outbuf[l]), "Predicates:\n", outbuf_size);
  // FIXME check and react to errors.
  l += l_sub;

  // FIXME any time we detect that *outbuf_size > 0 then
  //       switch to failure mode.

  const struct predicates_t * cursor;

  for (int i = 0; i < ATOM_DATABASE_SIZE; i++) {
    cursor = adb->atom_database[i];
    while (NULL != cursor && *outbuf_size > 0) {
      l_sub = predicate_str(cursor->predicate, outbuf_size, outbuf + l);
      // FIXME check and react to errors.
      l += l_sub;

      outbuf[(*outbuf_size)--, l++] = '\n';

      const struct clauses_t * clause_cursor = cursor->predicate->bodies;
      while (NULL != clause_cursor) {
        outbuf[(*outbuf_size)--, l++] = ' ';
        outbuf[(*outbuf_size)--, l++] = ' ';
        outbuf[(*outbuf_size)--, l++] = '*';
        outbuf[(*outbuf_size)--, l++] = ' ';
        l_sub = clause_to_str(clause_cursor->clause, outbuf_size, outbuf + l);
        // FIXME check and react to errors.
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

struct buffer_write_result *
Batom_database_str(struct atom_database_t * adb, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = buf_strcpy(dst, "Terms:");
  assert(is_ok_buffer_write_result(res));
  free(res);

  safe_buffer_replace_last(dst, '\n');

  res = Bterm_database_str(adb->tdb, dst);
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
      res = Bpredicate_str(cursor->predicate, dst);
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

        res = Bclause_to_str(clause_cursor->clause, dst);
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

size_t
predicate_str(const struct predicate_t * pred, size_t * outbuf_size, char * outbuf)
{
  size_t l = 0;
  size_t l_sub = my_strcpy(&(outbuf[l]), pred->predicate, outbuf_size);
  // FIXME check and react to errors.
  l += l_sub;

  outbuf[(*outbuf_size)--, l++] = '/';

  int l_sub_int = sprintf(&(outbuf[l]), "%u", pred->arity);
  if (l_sub_int <= 0) {
    // FIXME check and react to errors.
  }
  *outbuf_size -= strlen(&(outbuf[l]));
  l += (size_t)l_sub_int;

  assert(*outbuf_size > 0);

  return l;
}

struct buffer_write_result *
Bpredicate_str(const struct predicate_t * pred, struct buffer_info * dst)
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
clause_database_add(const struct clause_t * clause, struct atom_database_t * adb, void * cdl_add_error)
{
  adl_lookup_error_t adl_lookup_error;
  adl_add_error_t adl_add_error;
  struct predicate_t * record = NULL;
  bool success = atom_database_member(&clause->head, adb, &adl_lookup_error, &record);
  if (!success) {
    // FIXME elaborate this further
    ERR("Looking-up atom failed: %s\n", clause->head.predicate);
    cdl_add_error = 0; // FIXME set 'cdl_add_error'
    return false;
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
