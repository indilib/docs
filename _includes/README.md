# Includes Directory

This directory contains custom includes for the Jekyll site. These files are used to customize the appearance and behavior of the site.

## Files

- `head_custom.html`: Custom content to be included in the `<head>` section of the site. This file contains the JavaScript for the theme toggle functionality.
- `footer_custom.html`: Custom content to be included in the footer of the site. This file contains the theme toggle button.

## How It Works

The Just the Docs theme automatically includes these files if they exist. The `head_custom.html` file is included at the end of the `<head>` section, and the `footer_custom.html` file is included at the end of the footer.

The theme toggle functionality works by:

1. Adding a button in the footer via `footer_custom.html`
2. Adding JavaScript in the head via `head_custom.html` that:
   - Detects the user's preferred theme (light or dark)
   - Toggles between light and dark themes when the button is clicked
   - Saves the user's preference in localStorage
