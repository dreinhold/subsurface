#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dive.h"
#include "display.h"

static GtkWidget *divedate, *divetime, *depth, *duration;
static GtkTextBuffer *location, *notes;
static int location_changed = 1, notes_changed = 1;
static struct dive *buffered_dive;

static const char *weekday(int wday)
{
	static const char wday_array[7][4] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	return wday_array[wday];
}

static char *get_text(GtkTextBuffer *buffer)
{
	GtkTextIter start;
	GtkTextIter end;

	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	return gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
}

void flush_dive_info_changes(void)
{
	struct dive *dive = buffered_dive;

	if (!dive)
		return;

	if (location_changed) {
		g_free(dive->location);
		dive->location = get_text(location);
	}

	if (notes_changed) {
		g_free(dive->notes);
		dive->notes = get_text(notes);
	}
}

void update_dive_info(struct dive *dive)
{
	struct tm *tm;
	char buffer[80];
	char *text;

	flush_dive_info_changes();
	buffered_dive = dive;

	if (!dive) {
		gtk_label_set_text(GTK_LABEL(divedate), "no dive");
		gtk_label_set_text(GTK_LABEL(divetime), "");
		gtk_label_set_text(GTK_LABEL(depth), "");
		gtk_label_set_text(GTK_LABEL(duration), "");
		return;
	}

	tm = gmtime(&dive->when);
	snprintf(buffer, sizeof(buffer),
		"%s %02d/%02d/%04d",
		weekday(tm->tm_wday),
		tm->tm_mon+1, tm->tm_mday, tm->tm_year+1900);
	gtk_label_set_text(GTK_LABEL(divedate), buffer);

	snprintf(buffer, sizeof(buffer),
		"%02d:%02d:%02d",
		tm->tm_hour, tm->tm_min, tm->tm_sec);
	gtk_label_set_text(GTK_LABEL(divetime), buffer);

	snprintf(buffer, sizeof(buffer),
		"%d ft",
		to_feet(dive->maxdepth));
	gtk_label_set_text(GTK_LABEL(depth), buffer);

	snprintf(buffer, sizeof(buffer),
		"%d min",
		dive->duration.seconds / 60);
	gtk_label_set_text(GTK_LABEL(duration), buffer);

	text = dive->location ? : "";
	gtk_text_buffer_set_text(location, text, -1);
	text = dive->notes ? : "";
	gtk_text_buffer_set_text(notes, text, -1);
}

static GtkWidget *info_label(GtkWidget *box, const char *str)
{
	GtkWidget *label = gtk_label_new(str);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_RIGHT);
	gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);
	return label;
}

GtkWidget *dive_info_frame(void)
{
	GtkWidget *frame;
	GtkWidget *hbox;

	frame = gtk_frame_new("Dive info");
	gtk_widget_show(frame);

	hbox = gtk_hbox_new(TRUE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 3);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	divedate = info_label(hbox, "date");
	divetime = info_label(hbox, "time");
	depth = info_label(hbox, "depth");
	duration = info_label(hbox, "duration");

	return frame;
}

static GtkTextBuffer *text_entry(GtkWidget *box, const char *label, gboolean expand)
{
	GtkWidget *view;
	GtkTextBuffer *buffer;

	GtkWidget *frame = gtk_frame_new(label);

	gtk_box_pack_start(GTK_BOX(box), frame, expand, expand, 0);

	view = gtk_text_view_new ();
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	gtk_container_add(GTK_CONTAINER(frame), view);
	return buffer;
}

GtkWidget *extended_dive_info_frame(void)
{
	GtkWidget *frame;
	GtkWidget *vbox;

	frame = gtk_frame_new("Extended dive info");
	gtk_widget_show(frame);

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	location = text_entry(vbox, "Location", FALSE);
	notes = text_entry(vbox, "Notes", TRUE);

	/* Add extended info here: name, description, yadda yadda */
	update_dive_info(current_dive);
	return frame;
}