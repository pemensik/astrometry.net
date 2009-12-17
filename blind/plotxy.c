/*
  This file is part of the Astrometry.net suite.
  Copyright 2006, 2007 Dustin Lang, Keir Mierle and Sam Roweis.
  Copyright 2009 Dustin Lang.

  The Astrometry.net suite is free software; you can redistribute
  it and/or modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation, version 2.

  The Astrometry.net suite is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the Astrometry.net suite ; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/
#include <sys/param.h>

#include "plotxy.h"
#include "xylist.h"
#include "cairoutils.h"
#include "log.h"
#include "errors.h"
#include "sip_qfits.h"

const plotter_t plotter_xy = {
	"xy", 
	plot_xy_init,
	NULL,
	plot_xy_command,
	plot_xy_plot,
	plot_xy_free,
	NULL
};

void* plot_xy_init(plot_args_t* plotargs) {
	plotxy_t* args = calloc(1, sizeof(plotxy_t));
	args->ext = 1;
	args->scale = 1.0;
	return args;
}

int plot_xy_setsize(plot_args_t* pargs, plotxy_t* args) {
	xylist_t* xyls;
	xyls = xylist_open(args->fn);
	if (!xyls) {
		ERROR("Failed to open xylist from file \"%s\"", args->fn);
		return -1;
	}
	pargs->W = xylist_get_imagew(xyls);
	pargs->H = xylist_get_imageh(xyls);
	xylist_close(xyls);
	return 0;
}

int plot_xy_plot(const char* command, cairo_t* cairo,
				 plot_args_t* pargs, void* baton) {
	plotxy_t* args = (plotxy_t*)baton;
	// Plot it!
	xylist_t* xyls;
	starxy_t* xy;
	int Nxy;
	int i;

	if (!args->fn) {
		ERROR("No xylist filename given");
		return -1;
	}

	// Open xylist.
	xyls = xylist_open(args->fn);
	if (!xyls) {
		ERROR("Failed to open xylist from file \"%s\"", args->fn);
		return -1;
	}
	// we don't care about FLUX and BACKGROUND columns.
	xylist_set_include_flux(xyls, FALSE);
	xylist_set_include_background(xyls, FALSE);
	if (args->xcol)
		xylist_set_xname(xyls, args->xcol);
	if (args->ycol)
		xylist_set_yname(xyls, args->ycol);

	// Find number of entries in xylist.
	xy = xylist_read_field_num(xyls, args->ext, NULL);
	xylist_close(xyls);
	if (!xy) {
		ERROR("Failed to read FITS extension %i from file %s.\n", args->ext, args->fn);
		return -1;
	}
	Nxy = starxy_n(xy);
	// If N is specified, apply it as a max.
	if (args->nobjs)
		Nxy = MIN(Nxy, args->nobjs);

	// Shift and scale xylist entries.
	if (args->xoff != 0.0 || args->yoff != 0.0) {
		for (i=0; i<Nxy; i++) {
			starxy_setx(xy, i, starxy_getx(xy, i) - args->xoff);
			starxy_sety(xy, i, starxy_gety(xy, i) - args->yoff);
		}
	}
	if (args->scale != 1.0) {
		for (i=0; i<Nxy; i++) {
			starxy_setx(xy, i, args->scale * starxy_getx(xy, i));
			starxy_sety(xy, i, args->scale * starxy_gety(xy, i));
		}
	}

	// Transform through WCSes.
	if (args->wcs) {
		double ra, dec, x, y;
		assert(pargs->wcs);
		for (i=0; i<Nxy; i++) {
			bool ok;
			sip_pixelxy2radec(args->wcs,
							  starxy_getx(xy, i)+1, starxy_gety(xy, i)+1,
							  &ra, &dec);
			ok = sip_radec2pixelxy(pargs->wcs, ra, dec, &x, &y);
			starxy_setx(xy, i, x-1);
			starxy_sety(xy, i, y-1);
		}
	}

	if (args->bgrgba[3] != 0.0) {
		// Plot background.
		cairo_save(cairo);
		if (args->bglw)
			cairo_set_line_width(cairo, args->bglw);
		else
			cairo_set_line_width(cairo, pargs->lw + 2.0);
		cairo_set_rgba(cairo, args->bgrgba);
		for (i=args->firstobj; i<Nxy; i++) {
			double x = starxy_getx(xy, i) + 0.5;
			double y = starxy_gety(xy, i) + 0.5;
			cairoutils_draw_marker(cairo, pargs->marker, x, y, pargs->markersize);
			cairo_stroke(cairo);
		}
		cairo_restore(cairo);
	}

	// Plot markers.
	for (i=args->firstobj; i<Nxy; i++) {
		double x = starxy_getx(xy, i) + 0.5;
		double y = starxy_gety(xy, i) + 0.5;
		cairoutils_draw_marker(cairo, pargs->marker, x, y, pargs->markersize);
		cairo_stroke(cairo);
	}

	starxy_free(xy);
		
	return 0;
}

int plot_xy_set_bg(plotxy_t* args, const char* color) {
	return parse_color_rgba(color, args->bgrgba);
}

void plot_xy_set_xcol(plotxy_t* args, const char* col) {
	free(args->xcol);
	args->xcol = strdup_safe(col);
}

void plot_xy_set_ycol(plotxy_t* args, const char* col) {
	free(args->ycol);
	args->ycol = strdup_safe(col);
}

void plot_xy_set_filename(plotxy_t* args, const char* fn) {
	free(args->fn);
	args->fn = strdup_safe(fn);
}

int plot_xy_set_wcs_filename(plotxy_t* args, const char* fn) {
	free(args->wcs);
	args->wcs = sip_read_tan_or_sip_header_file_ext(fn, 0, NULL, FALSE);
	if (!args->wcs) {
		ERROR("Failed to read WCS file \"%s\"", fn);
		return -1;
	}
	return 0;
}

int plot_xy_command(const char* cmd, const char* cmdargs,
					plot_args_t* plotargs, void* baton) {
	plotxy_t* args = (plotxy_t*)baton;
	if (streq(cmd, "xy_file")) {
		plot_xy_set_filename(args, cmdargs);
	} else if (streq(cmd, "xy_ext")) {
		args->ext = atoi(cmdargs);
	} else if (streq(cmd, "xy_xcol")) {
		plot_xy_set_xcol(args, cmdargs);
	} else if (streq(cmd, "xy_ycol")) {
		plot_xy_set_ycol(args, cmdargs);
	} else if (streq(cmd, "xy_xoff")) {
		args->xoff = atof(cmdargs);
	} else if (streq(cmd, "xy_yoff")) {
		args->yoff = atof(cmdargs);
	} else if (streq(cmd, "xy_firstobj")) {
		args->firstobj = atoi(cmdargs);
	} else if (streq(cmd, "xy_nobjs")) {
		args->nobjs = atoi(cmdargs);
	} else if (streq(cmd, "xy_scale")) {
		args->scale = atof(cmdargs);
	} else if (streq(cmd, "xy_bgcolor")) {
		plot_xy_set_bg(args, cmdargs);
	} else if (streq(cmd, "xy_bglw")) {
		args->bglw = atof(cmdargs);
	} else if (streq(cmd, "xy_wcs")) {
		return plot_xy_set_wcs_filename(args, cmdargs);
	} else {
		ERROR("Did not understand command \"%s\"", cmd);
		return -1;
	}
	return 0;
}

void plot_xy_free(plot_args_t* plotargs, void* baton) {
	plotxy_t* args = (plotxy_t*)baton;
	free(args->wcs);
	free(args->xcol);
	free(args->ycol);
	free(args->fn);
	free(args);
}

