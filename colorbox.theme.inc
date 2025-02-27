<?php
/**
 * @file
 * Colorbox theme functions.
 */

/**
 * Returns HTML for a Colorbox image field formatter.
 *
 * @param array $variables
 *   An associative array containing:
 *   - item: An array of image data.
 *   - image_style: An optional image style.
 *   - path: An array containing the link 'path' and link 'options'.
 *
 * @return string
 *   An HTML string representing the themed output.
 *
 * @ingroup themeable
 */
function theme_colorbox_image_formatter($variables) {
  global $language;
  static $gallery_token = NULL;
  $item = $variables['item'];
  $entity_type = $variables['entity_type'];
  $entity = $variables['entity'];
  $field = $variables['field'];
  $settings = $variables['display_settings'];

  $image = array(
    'uri' => $item['uri'],
    'alt' => isset($item['alt']) ? $item['alt'] : '',
    'title' => isset($item['title']) ? $item['title'] : '',
    'style_name' => $settings['colorbox_node_style'],
  );

  if ($variables['delta'] == 0 && !empty($settings['colorbox_node_style_first'])) {
    $image['style_name'] = $settings['colorbox_node_style_first'];
  }

  if (isset($item['width']) && isset($item['height'])) {
    $image['width'] = $item['width'];
    $image['height'] = $item['height'];
  }

  if (isset($item['attributes'])) {
    $image['attributes'] = $item['attributes'];
  }

  // Allow image attributes to be overridden.
  if (isset($variables['item']['override']['attributes'])) {
    foreach (array('width', 'height', 'alt', 'title') as $key) {
      if (isset($variables['item']['override']['attributes'][$key])) {
        $image[$key] = $variables['item']['override']['attributes'][$key];
        unset($variables['item']['override']['attributes'][$key]);
      }
    }
    if (isset($image['attributes'])) {
      $image['attributes'] = $variables['item']['override']['attributes'] + $image['attributes'];
    }
    else {
      $image['attributes'] = $variables['item']['override']['attributes'];
    }
  }

  $entity_title = entity_label($entity_type, $entity);

  switch ($settings['colorbox_caption']) {
    case 'auto':
      // If the title is empty use alt or the entity title in that order.
      if (!empty($image['title'])) {
        $caption = $image['title'];
      }
      elseif (!empty($image['alt'])) {
        $caption = $image['alt'];
      }
      elseif (!empty($entity_title)) {
        $caption = $entity_title;
      }
      else {
        $caption = '';
      }
      break;

    case 'title':
      $caption = $image['title'];
      break;

    case 'alt':
      $caption = $image['alt'];
      break;

    case 'node_title':
      $caption = $entity_title;
      break;

    case 'custom':
      $caption = token_replace($settings['colorbox_caption_custom'], array($entity_type => $entity, 'file' => (object) $item), array('clear' => TRUE, 'language' => $language));
      break;

    default:
      $caption = '';
  }

  // Shorten the caption for the example styles or when caption shortening is
  // active.
  $colorbox_style = config_get('colorbox.settings', 'colorbox_style');
  $trim_length = config_get('colorbox.settings', 'colorbox_caption_trim_length');
  if (((strpos($colorbox_style, 'colorbox/example') !== FALSE) || config_get('colorbox.settings', 'colorbox_caption_trim')) && (backdrop_strlen($caption) > $trim_length)) {
    $caption = backdrop_substr($caption, 0, $trim_length - 5) . '...';
  }

  // Build the gallery id.
  list($id) = entity_extract_ids($entity_type, $entity);
  $entity_id = !empty($id) ? $entity_type . '-' . $id : 'entity-id';
  switch ($settings['colorbox_gallery']) {
    case 'post':
      $gallery_id = 'gallery-' . $entity_id;
      break;

    case 'page':
      $gallery_id = 'gallery-all';
      break;

    case 'field_post':
      $gallery_id = 'gallery-' . $entity_id . '-' . $field['field_name'];
      break;

    case 'field_page':
      $gallery_id = isset($field['field_name']) ? 'gallery-' . $field['field_name'] : 'gallery-page';
      break;

    case 'custom':
      $gallery_id = $settings['colorbox_gallery_custom'];
      break;

    default:
      $gallery_id = '';
  }


  // If gallery id is not empty add unique per-request token to avoid images
  // being added manually to galleries.
  $unique_token = config_get('colorbox.settings', 'colorbox_unique_token');
  if (!empty($gallery_id) && $unique_token) {
    // Check if gallery token has already been set, we need to reuse the token
    // for the whole request.
    if (is_null($gallery_token)) {
      // We use a short token since randomness is not critical.
      $gallery_token = backdrop_random_key(8);
    }
    $gallery_id = $gallery_id . '-' . $gallery_token;
  }

  if ($style_name = $settings['colorbox_image_style']) {
    $path = image_style_url($style_name, $image['uri']);
  }
  else {
    $path = file_create_url($image['uri']);
  }

  return theme('colorbox_imagefield', array(
    'image' => $image,
    'path' => $path,
    'title' => $caption,
    'gid' => $gallery_id,
  ));
}

/**
 * Returns HTML for an image using a specific Colorbox image style.
 *
 * @param array $variables
 *   An associative array containing:
 *   - image: image item as array.
 *   - path: The path of the image that should be displayed in the Colorbox.
 *   - title: The title text that will be used as a caption in the Colorbox.
 *   - gid: Gallery id for Colorbox image grouping.
 *
 * @return string
 *   An HTML string containing a link to the given path.
 *
 * @ingroup themeable
 */
function theme_colorbox_imagefield($variables) {
  $class = array('colorbox');

  if ($variables['image']['style_name'] == 'hide') {
    $image = '';
    $class[] = 'js-hide';
  }
  elseif (!empty($variables['image']['style_name'])) {
    $image = theme('image_style', $variables['image']);
  }
  else {
    $image = theme('image', $variables['image']);
  }

  $options = backdrop_parse_url($variables['path']);
  $options += array(
    'html' => TRUE,
    'attributes' => array(
      'title' => $variables['title'],
      'class' => $class,
      'data-colorbox-gallery' => $variables['gid'],
      'data-cbox-img-attrs' => '{"title": "' . $variables['image']['title'] . '", "alt": "' . $variables['image']['alt'] . '"}',
    ),
  );

  return l($image, $options['path'], $options);
}

/**
 * Preprocess variables for the colorbox-insert-image.tpl.php file.
 *
 * @param array $variables
 *   Associative array of variables for that template.
 */
function template_preprocess_colorbox_insert_image(&$variables) {
  $item = $variables['item'];
  $variables['file'] = file_load($item['fid']);
  $variables['style_name'] = $item['style_name'];
  $variables['width'] = isset($item['width']) ? $item['width'] : NULL;
  $variables['height'] = isset($item['height']) ? $item['height'] : NULL;

  // Determine dimensions of the image after the image style transformations.
  image_style_transform_dimensions($variables['style_name'], $variables);

  $class = array();
  if (!empty($variables['widget']['settings']['insert_class'])) {
    $class = explode(' ', $variables['widget']['settings']['insert_class']);
  }
  $class[] = 'image-' . $variables['style_name'];

  foreach ($class as $key => $value) {
    $class[$key] = backdrop_html_class($value);
  }

  $variables['class'] = implode(' ', $class);

  $variables['uri'] = image_style_path($variables['style_name'], $variables['file']->uri);
  $absolute = isset($variables['widget']['settings']['insert_absolute']) ? $variables['widget']['settings']['insert_absolute'] : NULL;
  $variables['url'] = insert_create_url($variables['uri'], $absolute, config_get('system.core', 'clean_url'));

  // http://drupal.org/node/1923336
  if (function_exists('image_style_path_token')) {
    $token_query = array(IMAGE_DERIVATIVE_TOKEN => image_style_path_token($variables['style_name'], $variables['file']->uri));
    $variables['url'] .= (strpos($variables['url'], '?') !== FALSE ? '&' : '?') . backdrop_http_build_query($token_query);
  }

  if ($style_name = config_get('colorbox.settings', 'colorbox_image_style')) {
    $variables['path'] = image_style_url($style_name, $variables['file']->uri);
  }
  else {
    $variables['path'] = file_create_url($variables['file']->uri);
  }

  $variables['gallery_id'] = '';
  switch (config_get('colorbox.settings', 'colorbox_insert_gallery')) {
    case 0:
    case 1:
    case 2:
      $variables['gallery_id'] = 'gallery-all';
      break;

    case 3:
      $variables['gallery_id'] = '';
      break;
  }
}
