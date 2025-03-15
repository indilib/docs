# Assets Directory

This directory contains assets used by the INDI documentation website, such as:

- Favicon
- Logo (if used)
- Custom images

## Adding Images

When adding images to the documentation, you can reference them using the following path format:

```markdown
![Image description](/assets/images/your-image.png)
```

Or with HTML for more control:

```html
<img src="/assets/images/your-image.png" alt="Image description" width="300" />
```

## Favicon

The favicon is configured in the `_config.yml` file and should be placed in this directory as `favicon.ico`.
