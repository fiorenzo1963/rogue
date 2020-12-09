/*
 * Read a scroll and let it happen
 *
 * @(#)scrolls.c	4.44 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * read_scroll:
 *	Read a scroll from the pack and do the appropriate thing
 */

void
read_scroll()
{
    THING *obj;
    PLACE *pp;
    int y, x;
    char ch;
    int i;
    struct room *cur_room;
    THING *fd_obj;
    static coord mp;

    static char id_type[S_ID_R_OR_S_5_4 + 1] = {
	    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	    POTION, SCROLL, WEAPON, ARMOR, R_OR_S
    };

    obj = get_item("read", SCROLL);
    if (obj == NULL)
	return;
    if (obj->o_type != SCROLL)
    {
	if (!terse)
	    msg("there is nothing on it to read");
	else
	    msg("nothing to read");
	return;
    }
    /*
     * Calculate the effect it has on the poor guy.
     */
    if (obj == cur_weapon)
	cur_weapon = NULL;
    /*
     * Get rid of the thing
     */
    obj = leave_pack(obj, FALSE);
    switch (obj->o_which) {
	/*
	 * generic
	 */
	case S_CONFUSE:
	    /*
	     * Scroll of monster confusion.  Give him that power.
	     */
	    player.t_flags |= CANHUH;
	    msg("your hands begin to glow %s", pick_color("red"));
	when S_MAP:
	    /*
	     * Scroll of magic mapping.
	     */
	    scr_info[S_MAP].oi_know = TRUE;
	    msg("oh, now this scroll has a map on it");
	    /*
	     * take all the things we want to keep hidden out of the window
	     */
	    for (y = 1; y < NUMLINES - 1; y++)
		for (x = 0; x < NUMCOLS; x++)
		{
		    pp = INDEX(y, x);
		    switch (ch = pp->p_ch)
		    {
			case DOOR:
			case STAIRS:
			    break;

			case '-':
			case '|':
			    if (!(pp->p_flags & F_REAL))
			    {
				ch = pp->p_ch = DOOR;
				pp->p_flags |= F_REAL;
			    }
			    break;

			case ' ':
			    if (pp->p_flags & F_REAL)
				goto def;
			    pp->p_flags |= F_REAL;
			    ch = pp->p_ch = PASSAGE;
			    /* FALLTHROUGH */

			case PASSAGE:
pass:
			    if (!(pp->p_flags & F_REAL))
				pp->p_ch = PASSAGE;
			    pp->p_flags |= (F_SEEN|F_REAL);
			    ch = PASSAGE;
			    break;

			case FLOOR:
			    if (pp->p_flags & F_REAL)
				ch = ' ';
			    else
			    {
				ch = TRAP;
				pp->p_ch = TRAP;
				pp->p_flags |= (F_SEEN|F_REAL);
			    }
			    break;

			default:
def:
			    if (pp->p_flags & F_PASS)
				goto pass;
			    ch = ' ';
			    break;
		    }
		    if (ch != ' ')
		    {
                        THING *tp;
			if ((tp = pp->p_monst) != NULL)
			    tp->t_oldch = ch;
			if (tp == NULL || !on(player, SEEMONST))
			    mvaddch(y, x, ch);
		    }
		}
	when S_HOLD:
	    /*
	     * Hold monster scroll.  Stop all monsters within two spaces
	     * from chasing after the hero.
	     */

	    ch = 0;
	    for (x = hero.x - 2; x <= hero.x + 2; x++)
		if (x >= 0 && x < NUMCOLS)
		    for (y = hero.y - 2; y <= hero.y + 2; y++)
			if (y >= 0 && y <= NUMLINES - 1)
                        {
                            THING *tp;
			    if ((tp = moat(y, x)) != NULL && on(*tp, ISRUN))
			    {
				tp->t_flags &= ~ISRUN;
				tp->t_flags |= ISHELD;
				ch++;
			    }
                        }
	    if (ch)
	    {
		addmsg("the monster");
		if (ch > 1)
		    addmsg("s around you");
		addmsg(" freeze");
		if (ch == 1)
		    addmsg("s");
		endmsg();
		scr_info[S_HOLD].oi_know = TRUE;
	    }
	    else
		msg("you feel a strange sense of loss");
	when S_SLEEP:
	    /*
	     * Scroll which makes you fall asleep
	     */
	    scr_info[S_SLEEP].oi_know = TRUE;
	    no_command += rnd(SLEEPTIME) + 4;
	    player.t_flags &= ~ISRUN;
	    msg("you fall asleep");
	when S_ARMOR:
	    if (cur_armor != NULL)
	    {
		cur_armor->o_arm--;
		cur_armor->o_flags &= ~ISCURSED;
		msg("your armor glows %s for a moment", pick_color("silver"));
	    }
	when S_SCARE:
	    /*
	     * Reading it is a mistake and produces laughter at her
	     * poor boo boo.
	     */
	    msg("you hear maniacal laughter in the distance");
	when S_TELEP:
	    /*
	     * Scroll of teleportation:
	     * Make him dissapear and reappear
	     */
	    {
		cur_room = proom;
		teleport();
		if (cur_room != proom)
		    scr_info[S_TELEP].oi_know = TRUE;
	    }
	when S_ENCH:
	    if (cur_weapon == NULL || cur_weapon->o_type != WEAPON)
		msg("you feel a strange sense of loss");
	    else
	    {
		cur_weapon->o_flags &= ~ISCURSED;
		if (rnd(2) == 0)
		    cur_weapon->o_hplus++;
		else
		    cur_weapon->o_dplus++;
		msg("your %s glows %s for a moment",
		    weap_info[cur_weapon->o_which].oi_name, pick_color("blue"));
	    }
	when S_CREATE:
	    /*
	     * Create a monster:
	     * First look in a circle around him, next try his room
	     * otherwise give up
	     */
	    i = 0;
	    for (y = hero.y - 1; y <= hero.y + 1; y++)
		for (x = hero.x - 1; x <= hero.x + 1; x++)
		    /*
		     * Don't put a monster in top of the player.
		     */
		    if (y == hero.y && x == hero.x)
			continue;
		    /*
		     * Or anything else nasty
		     */
		    else if (step_ok(ch = winat(y, x)))
		    {
			if (ch == SCROLL && find_obj(y, x)->o_which == S_SCARE)
				continue;
			else if (rnd(++i) == 0)
			{
			    mp.y = y;
			    mp.x = x;
			}
		    }
	    if (i == 0)
		msg("you hear a faint cry of anguish in the distance");
	    else
	    {
		THING *tp = new_item();
		new_monster(tp, randmonster(FALSE), &mp);
	    }
	when S_REMOVE:
	    uncurse(cur_armor);
	    uncurse(cur_weapon);
	    uncurse(cur_ring[LEFT]);
	    uncurse(cur_ring[RIGHT]);
	    msg(choose_str("you feel in touch with the Universal Onenes",
			   "you feel as if somebody is watching over you"));
	when S_AGGR:
	    /*
	     * This scroll aggravates all the monsters on the current
	     * level and sets them running towards the hero
	     */
	    aggravate();
	    msg("you hear a high pitched humming noise");
	when 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	    break;
	otherwise:
	    fatal("what a puzzling scroll #%d!", obj->o_which);
	    return;
    }

    if (ISVERSION_5_3()) {
	THING *op;

        /* handle version 5.3 scrolls */
        switch (obj->o_which) {
	case S_IDENT_5_3:
	    /*
	     * Identify, let him figure something out
	     */
	    scr_info[obj->o_which].oi_know = TRUE;
	    msg("this scroll is an %s scroll", scr_info[obj->o_which].oi_name);
	    whatis(TRUE, id_type[obj->o_which]);
	when S_GFIND_5_3:
	    /*
	     * Potion of gold detection
	     */
	    ch = FALSE;
	    wclear(hw);
	    for (op = lvl_obj; op != NULL; op = next(op))
		if (op->o_type == GOLD)
		{
		    ch = TRUE;
		    mvwaddch(hw, op->o_pos.y, op->o_pos.x, GOLD);
		}
	    if (ch)
	    {
	        scr_info[obj->o_which].oi_know = TRUE;
		show_win("You begin to feel greedy and you sense gold.--More--");
	    }
	    else
		msg("you feel a pull downward");
	when S_NOP_5_3:
	    msg("this scroll seems to be blank");
	when S_VORPAL_5_3:
	    /*
	     * Extra Vorpal Enchant Weapon
	     *     Give weapon +1,+1
	     *     Is extremely vorpal against one certain type of monster
	     *     Against this type (o_enemy) the weapon gets:
	     *		+4,+4
	     *		The ability to zap one such monster into oblivion
	     *
	     *     Some of these are cursed and if the rogue misses her saving
	     *     throw she will be forced to attack monsters of this type
	     *     whenever she sees one (not yet implemented)
	     *
	     * If he doesn't have a weapon I get to chortle again!
	     */
	    if (cur_weapon == NULL || cur_weapon->o_type != WEAPON)
		msg("you hear maniacal laughter in the distance");
	    else
	    {
		/*
		 * You aren't allowed to doubly vorpalize a weapon.
		 */
		if (cur_weapon->o_enemy != 0)
		{
                    THING *obj_w;
                    obj_w = leave_pack(cur_weapon, FALSE);
		    cur_weapon = NULL;
                    discard(obj_w);
		    msg("your %s vanishes in a puff of smoke",
			weap_info[cur_weapon->o_which].oi_name);
		}
		else
		{
		    cur_weapon->o_enemy = pick_mons();
		    cur_weapon->o_hplus++;
		    cur_weapon->o_dplus++;
		    cur_weapon->o_charges = 1;
		    msg("your %s gives off a flash of bright light.",
			weap_info[cur_weapon->o_which].oi_name);
		    /*
		     * Sometimes this is a mixed blessing ...
		     */
		    if (rnd(20) == 0)
		    {
			cur_weapon->o_flags |= ISCURSED;
			if (!save(VS_MAGIC))
			{
			    cur_weapon->o_flags |= ISREVEAL;
	                    scr_info[obj->o_which].oi_know = TRUE;
			    msg("you feel a sudden desire to kill %ss.",
				monsters[cur_weapon->o_enemy-'A'].m_name);
			}
		    }
		}
	    }
	when S_NOP2_5_3:
	case S_NOP3_5_3:
	case S_NOP4_5_3:
	    fatal("what a puzzling scroll #%d!", obj->o_which);
	    return;
	otherwise:
	    break;
	}

    } else {

        /*
	 * handle version 5.4 scrolls
	 */
        switch (obj->o_which) {
	when S_ID_POTION_5_4:
	case S_ID_SCROLL_5_4:
	case S_ID_WEAPON_5_4:
	case S_ID_ARMOR_5_4:
	case S_ID_R_OR_S_5_4:
	{
	    /*
	     * Identify, let him figure something out
	     */
	    scr_info[obj->o_which].oi_know = TRUE;
	    msg("this scroll is an %s scroll", scr_info[obj->o_which].oi_name);
	    whatis(TRUE, id_type[obj->o_which]);
	}
	when S_FDET_5_4:
	    /*
	     * Potion of food detection
	     */
	    ch = FALSE;
	    wclear(hw);
	    for (fd_obj = lvl_obj; fd_obj != NULL; fd_obj = next(fd_obj))
		if (fd_obj->o_type == FOOD)
		{
		    ch = TRUE;
		    wmove(hw, fd_obj->o_pos.y, fd_obj->o_pos.x);
		    waddch(hw, FOOD);
		}
	    if (ch)
	    {
		scr_info[S_FDET_5_4].oi_know = TRUE;
		show_win("Your nose tingles and you smell food.--More--");
	    }
	    else
		msg("your nose tingles");
	when S_PROTECT_5_4:
	    if (cur_armor != NULL)
	    {
		cur_armor->o_flags |= ISPROT;
		msg("your armor is covered by a shimmering %s shield",
		    pick_color("gold"));
	    }
	    else
		msg("you feel a strange sense of loss");
        otherwise:
	    break;
        }

    }

    look(TRUE);	/* put the result of the scroll on the screen */
    status();

    call_it(&scr_info[obj->o_which]);

    discard(obj);
}

/*
 * uncurse:
 *	Uncurse an item
 */

void
uncurse(THING *obj)
{
    if (obj != NULL)
	obj->o_flags &= ~ISCURSED;
}
