/*
 * File with various monster functions in it
 *
 * @(#)monsters.c	4.46 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <string.h>
#include "rogue.h"
#include <ctype.h>

/*
 * List of monsters in rough order of vorpalness
 */
static char lvl_mons_5_4[]  = "KEBSHIROZLCQANYFTWPXUMVGJD";
static char wand_mons_5_4[] = "KEBSH ROZ CQA Y TWP UMVGJ ";
static char lvl_mons_5_3[]  = "KJBSHEAOZGLCRQNYTWFIXUMVPD";
static char wand_mons_5_3[] = "KJBSH AOZG CRQ YTW IXU VP ";

#define lvl_mons  (ISVERSION_5_3() ? lvl_mons_5_3 : lvl_mons_5_4)
#define wand_mons (ISVERSION_5_3() ? wand_mons_5_3 : wand_mons_5_4)

/*
 * randmonster:
 *	Pick a monster to show up.  The lower the level,
 *	the meaner the monster.
 */
char
randmonster(bool wander)
{
    int d;
    char *mons;

    mons = (wander ? wand_mons : lvl_mons);
    do
    {
	d = level + (rnd(10) - 6);
	if (d < 0)
	    d = rnd(5);
	if (d > 25)
	    d = rnd(5) + 21;
    } while (mons[d] == ' ');
    return mons[d];
}

/*
 * new_monster:
 *	Pick a new monster and add it to the list
 */

void
new_monster(THING *tp, char type, coord *cp)
{
    struct monster *mp;
    int lev_add;

    if ((lev_add = level - AMULETLEVEL) < 0)
	lev_add = 0;
    attach(&mlist, tp);
    tp->t_type = type;
    tp->t_disguise = type;
    tp->t_pos = *cp;
    move(cp->y, cp->x);
    tp->t_oldch = CCHAR( inch() );
    tp->t_room = roomin(cp);
    moat(cp->y, cp->x) = tp;
    mp = &monsters[tp->t_type-'A'];
    tp->t_stats.s_lvl = mp->m_stats.s_lvl + lev_add;
    tp->t_stats.s_maxhp = tp->t_stats.s_hpt = roll(tp->t_stats.s_lvl, 8);
    tp->t_stats.s_arm = mp->m_stats.s_arm - lev_add;
    strcpy(tp->t_stats.s_dmg,mp->m_stats.s_dmg);
    tp->t_stats.s_str = mp->m_stats.s_str;
    tp->t_stats.s_exp = mp->m_stats.s_exp + lev_add * 10 + exp_add(tp);
    tp->t_flags = mp->m_flags;
    if (level > 29)
	tp->t_flags |= ISHASTE;
    tp->t_turn = TRUE;
    tp->t_pack = NULL;
    if (ISWEARING(R_AGGR))
	runto(cp);
    if (ISVERSION_5_3()) {
        if (type == 'M') {
	    switch (rnd(level > 25 ? 9 : 8))
	    {
	        when 0: tp->t_disguise = GOLD;
	        when 1: tp->t_disguise = POTION;
	        when 2: tp->t_disguise = SCROLL;
	        when 3: tp->t_disguise = STAIRS;
	        when 4: tp->t_disguise = WEAPON;
	        when 5: tp->t_disguise = ARMOR;
	        when 6: tp->t_disguise = RING;
	        when 7: tp->t_disguise = STICK;
	        when 8: tp->t_disguise = AMULET;
	    }
	}
    } else {
        if (type == 'X')
	    tp->t_disguise = rnd_thing();
    }
}

/*
 * expadd:
 *	Experience to add for this monster's level/hit points
 */
int
exp_add(THING *tp)
{
    int mod;

    if (tp->t_stats.s_lvl == 1)
	mod = tp->t_stats.s_maxhp / 8;
    else
	mod = tp->t_stats.s_maxhp / 6;
    if (tp->t_stats.s_lvl > 9)
	mod *= 20;
    else if (tp->t_stats.s_lvl > 6)
	mod *= 4;
    return mod;
}

/*
 * wanderer:
 *	Create a new wandering monster and aim it at the player
 */

void
wanderer()
{
    THING *tp;
    static coord cp;

    do
    {
	if (!find_floor((struct room *) NULL, &cp, MAXTRIES, TRUE, FF_NOHERO))
            return;
    } while (roomin(&cp) == proom);
    tp = new_item();
    new_monster(tp, randmonster(TRUE), &cp);
    if (on(player, SEEMONST))
    {
	standout();
	if (!on(player, ISHALU))
	    addch(tp->t_type);
	else
	    addch(rnd(26) + 'A');
	standend();
    }
    runto(&tp->t_pos);
#ifdef MASTER
    if (wizard)
	msg("started a wandering %s", monsters[tp->t_type-'A'].m_name);
#endif
}

/*
 * wake_monster:
 *	What to do when the hero steps next to a monster
 */
THING *
wake_monster(int y, int x)
{
    THING *tp;
    struct room *rp;
    char ch, *mname;
    char ch_mon;

#ifdef MASTER
    if ((tp = moat(y, x)) == NULL)
	msg("can't find monster in wake_monster");
#else
    tp = moat(y, x);
    if (tp == NULL) 	 	 
	endwin(), abort(); 
#endif
    ch = tp->t_type;
    /*
     * Every time he sees mean monster, it might start chasing him
     */
    if (!on(*tp, ISRUN) && rnd(3) != 0 && on(*tp, ISMEAN) && !on(*tp, ISHELD)
	&& !ISWEARING(R_STEALTH) && !on(player, ISLEVIT))
    {
	tp->t_dest = &hero;
	tp->t_flags |= ISRUN;
    }
    ch_mon = ISVERSION_5_3() ? 'U' : 'M';
    if (ch == ch_mon && !on(player, ISBLIND) && !on(player, ISHALU)
	&& !on(*tp, ISFOUND) && !on(*tp, ISCANC) && on(*tp, ISRUN))
    {
        rp = proom;
	if ((rp != NULL && !(rp->r_flags & ISDARK))
	    || dist(y, x, hero.y, hero.x) < LAMPDIST)
	{
	    tp->t_flags |= ISFOUND;
	    if (!save(VS_MAGIC))
	    {
		if (on(player, ISHUH))
		    lengthen(unconfuse, spread(HUHDURATION));
		else
		    fuse(unconfuse, 0, spread(HUHDURATION), AFTER);
		player.t_flags |= ISHUH;
		mname = set_mname(tp);
		addmsg("%s", mname);
		if (strcmp(mname, "it") != 0)
		    addmsg("'");
		msg("s gaze has confused you");
	    }
	}
    }
    /*
     * Let greedy ones guard gold
     */
    if (on(*tp, ISGREED) && !on(*tp, ISRUN))
    {
	tp->t_flags |= ISRUN;
	if (proom->r_goldval)
	    tp->t_dest = &proom->r_gold;
	else
	    tp->t_dest = &hero;
    }
    return tp;
}

/*
 * give_pack:
 *	Give a pack to a monster if it deserves one
 */

void
give_pack(THING *tp)
{
    if (level >= max_level && rnd(100) < monsters[tp->t_type-'A'].m_carry)
	attach(&tp->t_pack, new_thing());
}

/*
 * save_throw:
 *	See if a creature save against something
 */
int
save_throw(int which, THING *tp)
{
    int need;

    need = 14 + which - tp->t_stats.s_lvl / 2;
    return (roll(1, 20) >= need);
}

/*
 * save:
 *	See if he saves against various nasty things
 */
int
save(int which)
{
    if (which == VS_MAGIC)
    {
	if (ISRING(LEFT, R_PROTECT))
	    which -= cur_ring[LEFT]->o_arm;
	if (ISRING(RIGHT, R_PROTECT))
	    which -= cur_ring[RIGHT]->o_arm;
    }
    return save_throw(which, &player);
}
