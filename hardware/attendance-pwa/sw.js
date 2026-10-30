const VERSION = "14.0";
const CACHE_NAME = `attendance-manager-v${VERSION}`;
const APP_SHELL = [
  "./", "./index.html", "./manifest.json", "./css/main.css", "./css/layout.css", "./css/components.css", "./css/animations.css",
  "./js/app.js", "./js/serial.js", "./js/ch340.js", "./js/protocol.js", "./js/ui.js", "./js/state.js", "./js/utils.js",
  "./pages/dashboard.js", "./pages/users.js", "./pages/logs.js", "./pages/console.js", "./assets/icon.svg"
].map(url => `${url}?v=${VERSION}`);

self.addEventListener("install", event => event.waitUntil(caches.open(CACHE_NAME).then(cache => cache.addAll(APP_SHELL)).then(() => self.skipWaiting())));
self.addEventListener("activate", event => event.waitUntil(caches.keys().then(keys => Promise.all(keys.filter(key => key !== CACHE_NAME).map(key => caches.delete(key)))).then(() => self.clients.claim())));

self.addEventListener("fetch", event => {
  if (event.request.method !== "GET") return;
  event.respondWith(
    caches.match(event.request, { ignoreSearch: true }).then(cached => {
      const fetchPromise = fetch(event.request).then(networkResponse => {
        caches.open(CACHE_NAME).then(cache => cache.put(event.request, networkResponse.clone()));
        return networkResponse;
      }).catch(() => caches.match("./index.html", { ignoreSearch: true }));
      return cached || fetchPromise;
    })
  );
});
