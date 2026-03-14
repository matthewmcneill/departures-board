# Pico CSS Cheatsheet (Embedded Edition)

Pico CSS is a classless CSS framework, meaning it styles native HTML elements automatically. This is ideal for embedded systems as it minimizes the need for extra classes and reduces HTML size.

## Why Pico CSS?
- **Minimalistic**: ~10KB minified and gzipped.
- **Semantic**: Styles elements like `<nav>`, `<main>`, `<section>`, `<article>`, `<header>`, `<footer>`.
- **Responsive**: Built-in responsive grids.

## Basic Structure
```html
<!DOCTYPE html>
<html>
  <head>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/@picocss/pico@1/css/pico.min.css">
    <title>ESP32 Dashboard</title>
  </head>
  <body>
    <nav class="container-fluid">
      <ul><li><strong>Brand</strong></li></ul>
    </nav>
    <main class="container">
      <section>
        <article>
          <header>Sensor Data</header>
          Temperature: <span id="temp">--</span>°C
        </article>
      </section>
    </main>
  </body>
</html>
```

## Critical Components for ESP32
- **Grid System**: Use `<div class="grid">` for columns.
- **Forms**: Native `<input>`, `<select>`, and `<button>` are styled by default.
- **Progress**: `<progress>` is styled for status updates.
- **Modals**: Can be implemented using the `<dialog>` element.

> [!TIP]
> For the absolute smallest footprint, consider a local, even more stripped-down version of Pico or `Simple.css`.
