/*
 * new_level:
 *	Dig and draw a new level
 *
 * @(#)new_level.c	4.38 (Berkeley) 02/05/99
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

#define TREAS_ROOM 20	/* one chance in TREAS_ROOM for a treasure room */
#define TREAS_ROOM_ROOKIE 3 /* ditto for rookie mode */
#define MAXTREAS 12	/* maximum number of treasures in a treasure room */
#define MINTREAS 4	/* minimum number of treasures in a treasure room */

void
new_level()
{
    THING *tp;
    PLACE *pp;
    char *sp;
    int i;

    player.t_flags &= ~ISHELD;	/* unhold when you go down just in case */
    if (level > max_level)
	max_level = level;
    /*
     * Clean things off from last level
     */
    for (pp = places; pp < &places[MAXCOLS*MAXLINES]; pp++)
    {
	pp->p_ch = ' ';
	pp->p_flags = F_REAL;
	pp->p_monst = NULL;
    }
    clear();
    /*
     * Free up the monsters on the last level
     */
    for (tp = mlist; tp != NULL; tp = next(tp))
	free_list(&tp->t_pack);
    free_list(&mlist);
    /*
     * Throw away stuff left on the previous level (if anything)
     */
    free_list(&lvl_obj);
    do_rooms();				/* Draw rooms */
    do_passages();			/* Draw passages */
    no_food++;

    /*
     * setup t_room ptr
     */
    for (tp = mlist; tp != NULL; tp = next(tp))
	tp->t_room = roomin(&tp->t_pos);

    /*
     * Place first objects which we cannot do without
     */

    /*
     * Place the staircase down.
     */
    find_floor((struct room *) NULL, &stairs, NOLIMIT, FALSE, 0x0);
    chat(stairs.y, stairs.x) = STAIRS;
    seenstairs = FALSE;

    /*
     * Place amulet.
     */
    put_amulet();

    /*
     * Place hero.
     */
    find_floor((struct room *) NULL, &hero, NOLIMIT, TRUE, FF_NOTRAP);

    /*
     * Now place objects things which we can do without if we run out of space
     */

    populate_rooms();                   /* Populate rooms */

    put_things();			/* Place objects */

    /*
     * Place the traps
     */
    if (rnd(10) < level)
    {
	ntraps = rnd(level / 4) + 1;
	if (ntraps > MAXTRAPS)
	    ntraps = MAXTRAPS;
	i = ntraps;
	while (i--)
	{
	    /*
	     * not only wouldn't it be NICE to have traps in mazes
	     * (not that we care about being nice), since the trap
	     * number is stored where the passage number is, we
	     * can't actually do it.
	     */
	    do
	    {
		if (!find_floor((struct room *) NULL, &stairs, MAXTRIES, FALSE, FF_NOHERO))
                    goto skip;
	    } while (chat(stairs.y, stairs.x) != FLOOR);
	    sp = &flat(stairs.y, stairs.x);
	    *sp &= ~F_REAL;
	    *sp |= rnd(NTRAPS);
skip:
            ;
	}
    }

    enter_room(&hero);
    mvaddch(hero.y, hero.x, PLAYER);
    if (on(player, SEEMONST))
	turn_see(FALSE);
    if (on(player, ISHALU))
	visuals();
}

/*
 * rnd_room:
 *	Pick a room that is really there
 */
int
rnd_room()
{
    int rm;

    do
    {
	rm = rnd(MAXROOMS);
    } while (rooms[rm].r_flags & ISGONE);
    return rm;
}

/*
 * put_amulet:
 *	Put amulet
 */

void
put_amulet()
{
    /*
     * Once you have found the amulet, the only way to get new stuff is
     * go down into the dungeon.
     */
    if (amulet && level < max_level)
	return;

    /*
     * If he is really deep in the dungeon and he hasn't found the
     * amulet yet, put it somewhere on the ground
     */
    if (level >= AMULETLEVEL && !amulet)
    {
	THING *obj = new_item();
	attach(&lvl_obj, obj);
	obj->o_hplus = 0;
	obj->o_dplus = 0;
	strncpy(obj->o_damage,"0x0",sizeof(obj->o_damage));
        strncpy(obj->o_hurldmg,"0x0",sizeof(obj->o_hurldmg));
	obj->o_arm = 11;
	obj->o_type = AMULET;
	/*
	 * Put it somewhere
	 */
	find_floor((struct room *) NULL, &obj->o_pos, NOLIMIT, FALSE, FF_NOTRAP);
	chat(obj->o_pos.y, obj->o_pos.x) = AMULET;
    }
}

/*
 * put_things:
 *	Put potions and scrolls on this level
 */

void
put_things()
{
    int i;

    /*
     * Once you have found the amulet, the only way to get new stuff is
     * go down into the dungeon.
     */
    if (amulet && level < max_level)
	return;
    /*
     * check for treasure rooms, and if so, put it in.
     */
    if (rnd(rookie_mode ? TREAS_ROOM_ROOKIE : TREAS_ROOM) == 0)
	treas_room();

    /*
     * Do MAXOBJ attempts to put things on a level
     */
    for (i = 0; i < MAXOBJ; i++)
    {
	if (rnd(100) < 36)
	{
            coord mp;
	    if (find_floor((struct room *) NULL, &mp, MAXTRIES, FALSE, 0x0)) {
	        /*
	         * Pick a new object and link it in the list
	         */
                THING *obj = new_thing();
                obj->o_pos = mp;
	        attach(&lvl_obj, obj);
	        /*
	         * Put it somewhere
	         */
	        chat(obj->o_pos.y, obj->o_pos.x) = (char) obj->o_type;
            }
	}
    }
}

/*
 * treas_room:
 *	Add a treasure room
 */

void
treas_room()
{
    int nm;
    THING *tp;
    struct room *rp;
    int spots, num_monst;
    static coord mp;

    rp = &rooms[rnd_room()];
    spots = (rp->r_max.y - 2) * (rp->r_max.x - 2) - MINTREAS;
    if (spots > (MAXTREAS - MINTREAS))
	spots = (MAXTREAS - MINTREAS);
    num_monst = nm = rnd(spots) + MINTREAS;
    while (nm--)
    {
	if (find_floor(rp, &mp, 2 * MAXTRIES_TREAS, FALSE, 0x0)) {
	    tp = new_thing();
	    tp->o_pos = mp;
	    attach(&lvl_obj, tp);
	    chat(mp.y, mp.x) = (char) tp->o_type;
        }
    }

    /*
     * fill up room with monsters from the next level down
     */

    if ((nm = rnd(spots) + MINTREAS) < num_monst + 2)
	nm = num_monst + 2;
    spots = (rp->r_max.y - 2) * (rp->r_max.x - 2);
    if (nm > spots)
	nm = spots;
    level++;
    while (nm--)
    {
	spots = 0;
	if (find_floor(rp, &mp, MAXTRIES_TREAS, TRUE, FF_NOHERO))
	{
	    tp = new_item();
	    new_monster(tp, randmonster(FALSE), &mp);
	    tp->t_flags |= ISMEAN;	/* no sloughers in THIS room */
	    give_pack(tp);
	}
    }
    level--;
}
