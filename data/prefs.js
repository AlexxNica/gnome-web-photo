// don't allow opening file:/// URLs on pages from network sources (http, etc.)
pref("security.checkloaduri", true);

// disable sidebar What's Related, we don't use it
pref("browser.related.enabled", false);

// Work around for mozilla focus bugs
pref("mozilla.widget.raise-on-setfocus", false);

// disable sucky XUL ftp view, have nice ns4-like html page instead
pref("network.dir.generate_html", true);

// deactivate mailcap support, it breaks Gnome-based helper apps
pref("helpers.global_mailcap_file", "");
pref("helpers.private_mailcap_file", "");

// use the mozilla defaults for mime.types files to let mozilla guess proper
// Content-Type for file uploads instead of always falling back to
// application/octet-stream
pref("helpers.global_mime_types_file", "");
pref("helpers.private_mime_types_file", "");

// use google for keywords
pref("keyword.enabled", true);

// disable usless security warnings
pref("security.warn_entering_secure", false);
pref("security.warn_entering_secure.show_once", false);
pref("security.warn_leaving_secure", false);
pref("security.warn_submit_insecure", false);
pref("security.warn_submit_insecure.show_once", false);

// fonts
pref("font.size.unit", "pt");

// protocols
pref("network.protocol-handler.external.news", true);
pref("network.protocol-handler.external.mailto", true);
pref("network.protocol-handler.external.irc", true);
pref("network.protocol-handler.external.webcal", true);

// disable xpinstall
pref("xpinstall.enabled", false);

// disable typeahead find
pref("accessibility.typeaheadfind", false);

// disble null plugin
pref("plugin.default_plugin_disabled", true);

// disable password manager and form fill
pref("signon.rememberSignons", false);
pref("browser.formfill.enable", false);

// downgrade all cookies to session cookies, and accept them 
// only from originating site
pref("network.cookie.lifetimePolicy", 2);
pref("network.cookie.cookieBehavior", 2);

// disable disk cache
pref("browser.cache.disk.capacity", 0);

// delay onload until background images have loaded
pref("layout.fire_onload_after_image_background_loads", true);

// don't animate images or blink text
pref("image.animation_mode", "none");
pref("browser.blink_allowed", false);

// we don't want an image of an error page
pref("browser.xul.error_pages.enabled", false);

pref("general.useragent.firefox","Firefox/3.0");
