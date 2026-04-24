/*
 * Example 03: Custom Struct Types
 *
 * Demonstrates using a user-defined struct as the key/value rather
 * than simple int/string.  Here we build a skiplist of "Person"
 * records ordered by last name, then first name.
 *
 * Key concept: the comparison block can implement any ordering you
 * need -- multi-field, case-insensitive, reversed, etc.
 */
#include "sl.h"

/* A "Person" record with name and age.  The skiplist orders by
 * (last_name, first_name) lexicographically. */
struct person_node {
    char first_name[64];
    char last_name[64];
    int age;
    SKIPLIST_ENTRY(person) entry;
};

SKIPLIST_DECL(
    person, p_, entry,
    /* compare: order by (last_name, first_name) */
    {
        (void)list;
        (void)aux;
        int cmp = strcmp(a->last_name, b->last_name);
        if (cmp != 0)
            return cmp < 0 ? -1 : 1;
        cmp = strcmp(a->first_name, b->first_name);
        if (cmp != 0)
            return cmp < 0 ? -1 : 1;
        return 0;
    },
    /* free: nothing heap-allocated in our node */
    { (void)node; },
    /* update: overwrite the age field */
    {
        int *new_age = (int *)value;
        node->age = *new_age;
    },
    /* archive: shallow copy suffices (no heap pointers) */
    {
        memcpy(dest->first_name, src->first_name, sizeof(dest->first_name));
        memcpy(dest->last_name, src->last_name, sizeof(dest->last_name));
        dest->age = src->age;
        (void)rc;
    },
    /* sizeof */
    { bytes = sizeof(*node); })

/* Helper to create and insert a person. */
static int
insert_person(person_t *list, const char *first, const char *last, int age)
{
    person_node_t *node;
    int rc = p_skip_alloc_node_person(&node);
    if (rc)
        return rc;
    snprintf(node->first_name, sizeof(node->first_name), "%s", first);
    snprintf(node->last_name, sizeof(node->last_name), "%s", last);
    node->age = age;
    rc = p_skip_insert_person(list, node);
    if (rc)
        p_skip_free_node_person(list, node);
    return rc;
}

int
main(void)
{
    printf("=== Example 03: Custom Struct Types ===\n\n");

    person_t *list = (person_t *)malloc(sizeof(person_t));
    if (!list)
        return 1;
    p_skip_init_person(list);

    /* Insert people in arbitrary order. */
    insert_person(list, "Ada", "Lovelace", 36);
    insert_person(list, "Alan", "Turing", 41);
    insert_person(list, "Grace", "Hopper", 85);
    insert_person(list, "Dennis", "Ritchie", 70);
    insert_person(list, "Ken", "Thompson", 80);
    insert_person(list, "Brian", "Kernighan", 81);
    insert_person(list, "Barbara", "Liskov", 84);

    printf("Inserted %zu people.\n\n", p_skip_length_person(list));

    /* Iterate head-to-tail: should be sorted by (last, first). */
    printf("Sorted by (last name, first name):\n");
    person_node_t *cur;
    size_t i;
    SKIPLIST_FOREACH_H2T(person, p_, entry, list, cur, i)
    {
        printf("  %zu. %s %s, age %d\n", i + 1, cur->first_name, cur->last_name, cur->age);
    }

    /* Search for a specific person. */
    printf("\n--- Search for 'Alan Turing' ---\n");
    person_node_t query;
    memset(&query, 0, sizeof(query));
    snprintf(query.first_name, sizeof(query.first_name), "Alan");
    snprintf(query.last_name, sizeof(query.last_name), "Turing");
    person_node_t *found = p_skip_position_eq_person(list, &query);
    if (found)
        printf("Found: %s %s, age %d\n", found->first_name, found->last_name, found->age);
    else
        printf("Not found.\n");

    /* Search for someone who does not exist. */
    printf("\n--- Search for 'Linus Torvalds' ---\n");
    memset(&query, 0, sizeof(query));
    snprintf(query.first_name, sizeof(query.first_name), "Linus");
    snprintf(query.last_name, sizeof(query.last_name), "Torvalds");
    found = p_skip_position_eq_person(list, &query);
    if (found)
        printf("Found: %s %s, age %d\n", found->first_name, found->last_name, found->age);
    else
        printf("Not found.\n");

    /* Update Alan Turing's age using the update function. */
    printf("\n--- Update Turing's age to 42 ---\n");
    memset(&query, 0, sizeof(query));
    snprintf(query.first_name, sizeof(query.first_name), "Alan");
    snprintf(query.last_name, sizeof(query.last_name), "Turing");
    int new_age = 42;
    p_skip_update_person(list, &query, &new_age);
    found = p_skip_position_eq_person(list, &query);
    if (found)
        printf("Updated: %s %s, age %d\n", found->first_name, found->last_name, found->age);

    /* Clean up. */
    p_skip_free_person(list);
    free(list);

    printf("\nDone.\n");
    return 0;
}
