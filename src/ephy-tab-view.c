/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 *  Copyright © 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 *  This file is part of Epiphany.
 *
 *  Epiphany is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Epiphany is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Epiphany.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ephy-tab-view.h"

#include "ephy-desktop-utils.h"
#include "ephy-embed-utils.h"

struct _EphyTabView {
  GtkBin parent_instance;

  HdyTabView *tab_view;
  HdyTabPage *current_page;
};

G_DEFINE_TYPE (EphyTabView, ephy_tab_view, GTK_TYPE_BIN)

enum {
  PROP_0,
  PROP_N_PAGES,
  PROP_SELECTED_INDEX,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
notify_n_pages_cb (EphyTabView *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_N_PAGES]);
}

static void
notify_selected_page_cb (EphyTabView *self)
{
  HdyTabPage *page = hdy_tab_view_get_selected_page (self->tab_view);

  hdy_tab_page_set_needs_attention (page, FALSE);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SELECTED_INDEX]);
}

static void
secondary_icon_activated_cb (EphyTabView *self,
                             HdyTabPage  *page)
{
  EphyEmbed *embed = EPHY_EMBED (hdy_tab_page_get_child (page));
  EphyWebView *view = ephy_embed_get_web_view (embed);
  gboolean muted = webkit_web_view_get_is_muted (WEBKIT_WEB_VIEW (view));

  webkit_web_view_set_is_muted (WEBKIT_WEB_VIEW (view), !muted);
}

static void
setup_menu_cb (EphyTabView *self,
               HdyTabPage  *page)
{
  self->current_page = page;
}

static HdyTabPage *
get_current_page (EphyTabView *self)
{
  if (self->current_page)
    return self->current_page;

  return hdy_tab_view_get_selected_page (self->tab_view);
}

static void
ephy_tab_view_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  EphyTabView *self = EPHY_TAB_VIEW (object);

  switch (prop_id) {
    case PROP_N_PAGES:
      g_value_set_int (value, ephy_tab_view_get_n_pages (self));
      break;

    case PROP_SELECTED_INDEX:
      g_value_set_int (value, ephy_tab_view_get_selected_index (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
ephy_tab_view_class_init (EphyTabViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = ephy_tab_view_get_property;

  properties[PROP_N_PAGES] =
    g_param_spec_int ("n-pages",
                      "Number of pages",
                      "The number of pages in the tab view",
                      0,
                      G_MAXINT,
                      0,
                      (G_PARAM_READABLE |
                       G_PARAM_STATIC_STRINGS |
                       G_PARAM_EXPLICIT_NOTIFY));

  properties[PROP_SELECTED_INDEX] =
    g_param_spec_int ("selected-index",
                      "Selected index",
                      "The index of the currently selected page",
                      0,
                      G_MAXINT,
                      0,
                      (G_PARAM_READABLE |
                       G_PARAM_STATIC_STRINGS |
                       G_PARAM_EXPLICIT_NOTIFY));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
ephy_tab_view_init (EphyTabView *self)
{
  self->tab_view = HDY_TAB_VIEW (hdy_tab_view_new ());
  gtk_widget_show (GTK_WIDGET (self->tab_view));

  g_object_set_data (G_OBJECT (self->tab_view), "ephy-tab-view", self);

  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (self->tab_view));

  g_signal_connect_object (self->tab_view,
                           "notify::n-pages",
                           G_CALLBACK (notify_n_pages_cb),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (self->tab_view,
                           "notify::selected-page",
                           G_CALLBACK (notify_selected_page_cb),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (self->tab_view,
                           "setup-menu",
                           G_CALLBACK (setup_menu_cb),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (self->tab_view,
                           "secondary-icon-activated",
                           G_CALLBACK (secondary_icon_activated_cb),
                           self,
                           G_CONNECT_SWAPPED);
}

EphyTabView *
ephy_tab_view_new (void)
{
  return g_object_new (EPHY_TYPE_TAB_VIEW, NULL);
}

void
ephy_tab_view_next (EphyTabView *self)
{
  hdy_tab_view_select_next_page (self->tab_view);
}

void
ephy_tab_view_pin (EphyTabView *self)
{
  hdy_tab_view_set_page_pinned (self->tab_view, get_current_page (self), TRUE);
}

void
ephy_tab_view_unpin (EphyTabView *self)
{
  hdy_tab_view_set_page_pinned (self->tab_view, get_current_page (self), FALSE);
}

void
ephy_tab_view_close (EphyTabView *self,
                     GtkWidget   *widget)
{
  HdyTabPage *page = hdy_tab_view_get_page (self->tab_view, widget);

  hdy_tab_view_close_page (self->tab_view, page);
}

void
ephy_tab_view_close_selected (EphyTabView *self)
{
  hdy_tab_view_close_page (self->tab_view, get_current_page (self));
}

void
ephy_tab_view_close_left (EphyTabView *self)
{
  hdy_tab_view_close_pages_before (self->tab_view, get_current_page (self));
}

void
ephy_tab_view_close_right (EphyTabView *self)
{
  hdy_tab_view_close_pages_after (self->tab_view, get_current_page (self));
}

void
ephy_tab_view_close_other (EphyTabView *self)
{
  hdy_tab_view_close_other_pages (self->tab_view, get_current_page (self));
}

void
ephy_tab_view_foreach (EphyTabView *self,
                       GtkCallback  callback,
                       gpointer     user_data)
{
  gint i, n;

  n = hdy_tab_view_get_n_pages (self->tab_view);

  for (i = 0; i < n; i++) {
    HdyTabPage *page = hdy_tab_view_get_nth_page (self->tab_view, i);

    callback (hdy_tab_page_get_child (page), user_data);
  }
}

gint
ephy_tab_view_get_n_pages (EphyTabView *self)
{
  return hdy_tab_view_get_n_pages (self->tab_view);
}

gint
ephy_tab_view_get_selected_index (EphyTabView *self)
{
  HdyTabPage *page = hdy_tab_view_get_selected_page (self->tab_view);

  return hdy_tab_view_get_page_position (self->tab_view, page);
}

GtkWidget *
ephy_tab_view_get_nth_page (EphyTabView *self,
                            gint         index)
{
  HdyTabPage *page = hdy_tab_view_get_nth_page (self->tab_view, index);

  return hdy_tab_page_get_child (page);
}

void
ephy_tab_view_select_nth_page (EphyTabView *self,
                               gint         index)
{
  HdyTabPage *page = hdy_tab_view_get_nth_page (self->tab_view, index);

  hdy_tab_view_set_selected_page (self->tab_view, page);
}

gboolean
ephy_tab_view_select_page (EphyTabView *self,
                           GtkWidget   *widget)
{
  HdyTabPage *page = hdy_tab_view_get_page (self->tab_view, widget);

  if (page)
    hdy_tab_view_set_selected_page (self->tab_view, page);

  return page != NULL;
}

GtkWidget *
ephy_tab_view_get_selected_page (EphyTabView *self)
{
  HdyTabPage *page = hdy_tab_view_get_selected_page (self->tab_view);

  return hdy_tab_page_get_child (page);
}

HdyTabView *
ephy_tab_view_get_tab_view (EphyTabView *self)
{
  return self->tab_view;
}

GList *
ephy_tab_view_get_pages (EphyTabView *self)
{
  GList *list = NULL;
  gint i, n;

  n = hdy_tab_view_get_n_pages (self->tab_view);

  for (i = 0; i < n; i++) {
    HdyTabPage *page = hdy_tab_view_get_nth_page (self->tab_view, i);
    GtkWidget *content = hdy_tab_page_get_child (page);

    list = g_list_prepend (list, content);
  }

  return g_list_reverse (list);
}

gboolean
ephy_tab_view_get_is_pinned (EphyTabView *self,
                             GtkWidget   *widget)
{
  HdyTabPage *page = hdy_tab_view_get_page (self->tab_view, widget);

  return hdy_tab_page_get_pinned (page);
}

static void
update_title_cb (HdyTabPage *page)
{
  EphyEmbed *embed = EPHY_EMBED (hdy_tab_page_get_child (page));
  EphyWebView *view = ephy_embed_get_web_view (embed);
  const gchar *title = ephy_embed_get_title (embed);
  const gchar *address;

  if (!ephy_embed_has_load_pending (embed) &&
      !hdy_tab_page_get_selected (page) &&
      hdy_tab_page_get_pinned (page))
    hdy_tab_page_set_needs_attention (page, TRUE);

  if (title && strlen (title)) {
    hdy_tab_page_set_title (page, title);

    return;
  }

  address = ephy_web_view_get_display_address (view);

  if (ephy_web_view_is_loading (view) &&
      !ephy_embed_utils_is_no_show_address (address))
    hdy_tab_page_set_title (page, address);
}

static void
update_icon_cb (HdyTabPage *page)
{
  EphyEmbed *embed = EPHY_EMBED (hdy_tab_page_get_child (page));
  EphyWebView *view = ephy_embed_get_web_view (embed);
  g_autoptr (GIcon) icon = G_ICON (ephy_web_view_get_icon (view));
  const char *uri, *favicon_name;

  if (icon) {
    hdy_tab_page_set_icon (page, g_steal_pointer (&icon));

    return;
  }

  uri = webkit_web_view_get_uri (WEBKIT_WEB_VIEW (view));
  favicon_name = ephy_get_fallback_favicon_name (uri, EPHY_FAVICON_TYPE_NO_MISSING_PLACEHOLDER);

  if (favicon_name)
    icon = g_themed_icon_new (favicon_name);

  hdy_tab_page_set_icon (page, icon);
}

static void
update_secondary_icon_cb (HdyTabPage *page)
{
  EphyEmbed *embed = EPHY_EMBED (hdy_tab_page_get_child (page));
  EphyWebView *view = ephy_embed_get_web_view (embed);
  GIcon *icon = NULL;

  if (webkit_web_view_is_playing_audio (WEBKIT_WEB_VIEW (view))) {
    if (webkit_web_view_get_is_muted (WEBKIT_WEB_VIEW (view)))
      icon = G_ICON (g_themed_icon_new ("ephy-audio-muted-symbolic"));
    else
      icon = G_ICON (g_themed_icon_new ("ephy-audio-playing-symbolic"));
  }

  hdy_tab_page_set_secondary_icon (page, icon);
}

gint
ephy_tab_view_add_tab (EphyTabView *self,
                       EphyEmbed   *embed,
                       gint         position,
                       gboolean     jump_to)
{
  HdyTabPage *page;
  EphyWebView *view;

  if (position < 0)
    page = hdy_tab_view_append (self->tab_view, GTK_WIDGET (embed));
  else
    page = hdy_tab_view_insert (self->tab_view, GTK_WIDGET (embed), position);

  if (jump_to)
    hdy_tab_view_set_selected_page (self->tab_view, page);

  view = ephy_embed_get_web_view (embed);

  hdy_tab_page_set_secondary_icon_activatable (page, TRUE);

  g_object_bind_property (view, "is-loading", page, "loading", G_BINDING_SYNC_CREATE);

  g_signal_connect_object (embed, "notify::title",
                           G_CALLBACK (update_title_cb), page,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (view, "notify::display-address",
                           G_CALLBACK (update_title_cb), page,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (view, "notify::icon",
                           G_CALLBACK (update_icon_cb), page,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (view, "notify::uri",
                           G_CALLBACK (update_icon_cb), page,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (view, "notify::is-playing-audio",
                           G_CALLBACK (update_secondary_icon_cb), page,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (view, "notify::is-muted",
                           G_CALLBACK (update_secondary_icon_cb), page,
                           G_CONNECT_SWAPPED);

  update_title_cb (page);
  update_icon_cb (page);
  update_secondary_icon_cb (page);

  return hdy_tab_view_get_page_position (self->tab_view, page);
}

GtkWidget *
ephy_tab_view_get_current_page (EphyTabView *self)
{
  return hdy_tab_page_get_child (get_current_page (self));
}
