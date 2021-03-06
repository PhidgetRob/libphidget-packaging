#include "../stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ptree.h"

struct ptree_node {
	void *pn_value;
	struct ptree_node *pn_parent;
	struct ptree_node *pn_left;
	struct ptree_node *pn_right;
};

static int
_walk_to(void *v, struct ptree_node **startp, struct ptree_node ***pp,
    int (*cmp)(const void *v1, const void *v2))
{
	struct ptree_node **nextp;
	int c = -1;

	nextp = startp;
	while (*nextp) {
		c = cmp(v, (*nextp)->pn_value);
		*startp = *nextp;
		if (c == 0)
			break;
		if (c < 0)
			nextp = &(*nextp)->pn_left;
		else
			nextp = &(*nextp)->pn_right;
		if (pp)
			*pp = nextp;
	}

	return c;
}

//
int
ptree_replace(void *v, ptree_node_t **rootp, int (*cmp)(const void *v1,
    const void *v2), void **oldval)
{
	struct ptree_node **parentpp;
	struct ptree_node *parentp;
	struct ptree_node *pn;
	int c;

	parentp = *rootp;
	parentpp = rootp;
	//if we find v in the tree, replace it
	if ((c = _walk_to(v, &parentp, &parentpp, cmp)) == 0) {
		if (oldval)
			*oldval = parentp->pn_value;
		parentp->pn_value = v;
		return 1;
	}
	//otherwise, add it to the tree
	if (!(pn = malloc(sizeof (*pn))))
		return 0;
	memset(pn, 0, sizeof (*pn));
	pn->pn_value = v;
	pn->pn_parent = parentp;
	*parentpp = pn;
	if (oldval)
		*oldval = 0;

	return 1;
}

int
ptree_contains(void *v, ptree_node_t *parentp, int (*cmp)(const void *v1,
    const void *v2), void **nodeval)
{
	int c;

	if ((c = _walk_to(v, &parentp, NULL, cmp)) == 0) {
		if (nodeval)
			*nodeval = parentp->pn_value;
		return 1;
	}
	if (nodeval)
		*nodeval = 0;
	return 0;
}

static ptree_walk_res_t
_visit(struct ptree_node *pn, int level, void *arg1, void *arg2)
{
	ptree_walk_res_t (*func)(const void *, int, void *, void *) = arg1;

	return func(pn->pn_value, level, arg2, pn);
}

static ptree_node_t *
_find_min(ptree_node_t *pn, int *levelp)
{
	while (pn->pn_left) {
		pn = pn->pn_left;
		if (levelp)
			(*levelp)++;
	}
	return pn;
}

static ptree_node_t *
_find_succ(ptree_node_t *pn, int *levelp)
{
	if (pn->pn_right) {
		pn = pn->pn_right;
		if (levelp)
			(*levelp)++;
		while (pn->pn_left) {
			pn = pn->pn_left;
			if (levelp)
				(*levelp)++;
		}
	} else {
		while (pn->pn_parent && pn->pn_parent->pn_right == pn) {
			pn = pn->pn_parent;
			if (levelp)
				(*levelp)--;
		}
		pn = pn->pn_parent;
		if (levelp)
			(*levelp)--;
	}
		
	return pn;
}

static int
_walk_int(struct ptree_node *pn, ptree_order_t order, int level,
    ptree_walk_res_t (*func)(struct ptree_node *, int level, void *, void *),
    void *arg1, void *arg2)
{
	ptree_walk_res_t res;
	ptree_node_t *next;

	if (!pn)
		return PTREE_WALK_CONTINUE;
	if (order == PTREE_INORDER) {
		int nlevel;

		pn = _find_min(pn, &level);
		while (pn) {
			nlevel = level;
			next = _find_succ(pn, &nlevel);
			if ((res = func(pn, level, arg1, arg2)) !=
			    PTREE_WALK_CONTINUE)
				return res;
			level = nlevel;
			if (level < 0)
				level = 0;
			pn = next;
		}
		return PTREE_WALK_CONTINUE;
	}
	if (order == PTREE_PREORDER && (res = func(pn, level, arg1, arg2)) !=
	    PTREE_WALK_CONTINUE)
		return res;
	if ((res = _walk_int(pn->pn_left, order, level + 1, func, arg1,
	    arg2)) != PTREE_WALK_CONTINUE)
		return res;
	if ((res = _walk_int(pn->pn_right, order, level + 1, func, arg1,
	    arg2)) != PTREE_WALK_CONTINUE)
		return res;
	if (order == PTREE_POSTORDER && (res = func(pn, level, arg1, arg2))
	    != PTREE_WALK_CONTINUE)
		return res;
	return PTREE_WALK_CONTINUE;
}

static void
_remove_node(ptree_node_t **rootp, ptree_node_t *pn, void **oldval)
{
	ptree_node_t **pp;
	ptree_node_t **predp;
	ptree_node_t *pred;

	if (!pn->pn_parent) {
		assert(rootp && pn == *rootp);
		pp = rootp;
	} else {
		if (pn->pn_parent->pn_left == pn)
			pp = &pn->pn_parent->pn_left;
		else
			pp = &pn->pn_parent->pn_right;
	}
	if (!pn->pn_left) {
		if (pp)
			*pp = pn->pn_right;
		if (pn->pn_right)
			pn->pn_right->pn_parent = pn->pn_parent;
	} else if (!pn->pn_right) {
		if (pp)
			*pp = pn->pn_left;
		if (pn->pn_left)
			pn->pn_left->pn_parent = pn->pn_parent;
	} else {
		for (predp = &pn->pn_left; (*predp)->pn_right; )
			predp = &(*predp)->pn_right;
		pred = *predp;
		*predp = NULL;
		if (pp)
			*pp = pred;
		pred->pn_parent = pn->pn_parent;
		pred->pn_left = pn->pn_left;
		pred->pn_right = pn->pn_right;
		if (pn->pn_left)
			pn->pn_left->pn_parent = pred;
		if (pn->pn_right)
			pn->pn_right->pn_parent = pred;
	}
	if (oldval)
		*oldval = pn->pn_value;
	free(pn); pn = NULL;
}

int
ptree_inorder_walk_remove(ptree_node_t **rootp, void **oldval,
    void *pn)
{
	assert(pn);
	if (!pn)
		return 0;
	_remove_node(rootp, pn, oldval);
	return 1;
}

ptree_walk_res_t
ptree_walk(ptree_node_t *start, ptree_order_t order,
    ptree_walk_res_t (*func)(const void *v1, int level, void *arg,
    void *ptree_inorder_walking_remove_arg), void *arg)
{
	return _walk_int((struct ptree_node *)start, order, 0, _visit,
	    (void *)func, arg);
}

ptree_walk_res_t
ptree_clear_func(struct ptree_node *pn, int level, void *arg1, void *arg2)
{
	free(pn); pn = NULL;

	return PTREE_WALK_CONTINUE;
}

void
ptree_clear(ptree_node_t **rootp)
{
	_walk_int(*rootp, PTREE_POSTORDER, 0, ptree_clear_func, 0, 0);
	*rootp = 0;
}

int
ptree_remove(void *v, ptree_node_t **rootp, int (*cmp)(const void *v1,
    const void *v2), void **oldval)
{
	struct ptree_node *cur;

	cur = *rootp;
	if (_walk_to(v, &cur, NULL, cmp) != 0)
		return 0;
	_remove_node(rootp, cur, oldval);
	return 1;
}
