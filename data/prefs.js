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
pref("font.size.fixed.ar", 10);
pref("font.size.fixed.x-baltic", 10);
pref("font.size.fixed.x-central-euro", 10);
pref("font.size.fixed.x-cyrillic", 10);
pref("font.size.fixed.x-devanagari", 10);
pref("font.size.fixed.el", 10);
pref("font.size.fixed.he", 10);
pref("font.size.fixed.ja", 10);
pref("font.size.fixed.ko", 10);
pref("font.size.fixed.zh-CN", 10);
pref("font.size.fixed.x-tamil", 10);
pref("font.size.fixed.th", 10);
pref("font.size.fixed.zh-TW", 10);
pref("font.size.fixed.zh-HK", 10);
pref("font.size.fixed.tr", 10);
pref("font.size.fixed.x-unicode", 10);
pref("font.size.fixed.x-western", 10);
pref("font.size.variable.ar", 11);
pref("font.size.variable.x-baltic", 11);
pref("font.size.variable.x-central-euro", 11);
pref("font.size.variable.x-cyrillic", 11);
pref("font.size.variable.x-devanagari", 11);
pref("font.size.variable.el", 11);
pref("font.size.variable.he", 11);
pref("font.size.variable.ja", 11);
pref("font.size.variable.ko", 11);
pref("font.size.variable.zh-CN", 11);
pref("font.size.variable.x-tamil", 11);
pref("font.size.variable.th", 11);
pref("font.size.variable.zh-TW", 11);
pref("font.size.variable.zh-HK", 11);
pref("font.size.variable.tr", 11);
pref("font.size.variable.x-unicode", 11);
pref("font.size.variable.x-western", 11);

// protocols
pref("network.protocol-handler.external.news", true);
pref("network.protocol-handler.external.mailto", true);
pref("network.protocol-handler.external.irc", true);
pref("network.protocol-handler.external.webcal", true);

// disable xpinstall
pref("xpinstall.enabled", false);

// disable typeahead find
pref("accessibility.typeaheadfind", false);

// use system proxy prefs
pref("config.use_system_prefs", true);

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

// we don't want an image of an error page
pref("browser.xul.error_pages.enabled", false);

