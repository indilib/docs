# Sass Directory

This directory contains custom Sass files for the Jekyll site. These files are used to customize the appearance of the site.

## Structure

- `custom/`: Directory containing custom Sass files
  - `custom.scss`: Custom styles for the site

## How It Works

The Just the Docs theme automatically compiles these files if they exist. The `custom.scss` file is included at the end of the theme's main Sass file, allowing you to override or extend the theme's styles.

The Sass configuration is defined in the `_config.yml` file:

```yaml
sass:
  sass_dir: _sass
  style: compressed
```

## Adding Custom Styles

To add custom styles, you can edit the `custom/custom.scss` file. This file is already set up to include styles for the theme toggle button.

You can also add new Sass files to the `custom/` directory and import them in the `custom.scss` file:

```scss
@import "custom/my-new-styles";
```

Just make sure to create a corresponding `_sass/custom/my-new-styles.scss` file.
