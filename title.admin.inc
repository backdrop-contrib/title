<?php

/**
 * @file
 * Admin page callbacks for the Title module.
 */

/**
 * Provide settings to enable title field.
 */
function title_form_field_ui_overview(&$form, &$form_state) {
  $entity_info = entity_get_info($form['#entity_type']);

  if (!empty($entity_info['field replacement'])) {
    $field_replacement_info = $entity_info['field replacement'];
    $admin_path = _field_ui_bundle_admin_path($form['#entity_type'], $form['#bundle']);

    foreach (element_children($form['fields']) as $field_name) {
      if (isset($field_replacement_info[$field_name])) {
        $form['fields'][$field_name]['delete'] = array(
          '#prefix' => '
            <div class="dropbutton-wrapper dropbutton-processed">
              <div class="dropbutton-widget">
                <ul class="dropbutton">
                  <li class="edit odd first dropbutton-action">
          ',
          '#type' => 'link',
          '#title' => t('replace'),
          '#href' => $admin_path . '/fields/replace/' . $field_name,
          '#options' => array('attributes' => array('title' => t('Replace %field with a customizable field instance that can be translated.', array('%field' => $field_name)))),
          '#suffix' => '
                  </li>
                </ul>
              </div>
            </div>
          ',
        );
      }
    }
  }
}

/**
 * Generate a field replacement form.
 */
function title_field_replacement_form($form, $form_state, $entity_type, $bundle, $field_name) {
  $bundle_name = field_extract_bundle($entity_type, $bundle);
  $entity_info = entity_get_info($entity_type);
  $info = $entity_info['field replacement'][$field_name];
  $instance = field_info_instance($entity_type, $info['field']['field_name'], $bundle_name);
  $enabled = !empty($instance);

  $form['#entity_type'] = $entity_type;
  $form['#bundle'] = $bundle_name;
  $form['#field_name'] = $field_name;

  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Replace %field with a field instance', array('%field' => $field_name)),
    '#description' => t('If this is enabled the %field will be replaced with a customizable field that can be translated.', array('%field' => $field_name)),
    '#default_value' => $enabled,
    '#disabled' => $enabled,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save settings'));
  return $form;
}

/**
 * Process field replacement form subissions.
 */
function title_field_replacement_form_submit($form, &$form_state) {
  if ($form_state['values']['enabled'] != $form['enabled']['#default_value']) {
    if (title_field_replacement_toggle($form['#entity_type'], $form['#bundle'], $form['#field_name'])) {
      backdrop_set_message(t('%field replaced with a field instance.', array('%field' => $form['#field_name'])));
      title_field_replacement_batch_set($form['#entity_type'], $form['#bundle'], $form['#field_name']);
    }
    else {
      backdrop_set_message(t('Field replacement removed.'));
    }
  }
  $form_state['redirect'] = _field_ui_bundle_admin_path($form['#entity_type'], $form['#bundle']) . '/fields';
}

/**
 * Form settings for automated title_field attachement.
 */
function title_admin_settings_form() {
  $form['tabs'] = array(
    '#type' => 'vertical_tabs',
  );

  foreach (entity_get_info() as $entity_type => $info) {
    if (empty($info['field replacement'])) {
      continue;
    }

    $form['settings']['title_' . $entity_type] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#tree' => TRUE,
      '#group' => 'tabs',
      '#title' => $info['label'],
    );

    $options = array();
    foreach (array_keys($info['field replacement']) as $replacement) {
      $options[$replacement] = backdrop_ucfirst($replacement);
    }

    //$default = variable_get('title_' . $entity_type, array());
    $default = config_get('title.settings', 'title_' . $entity_type);
    $form['settings']['title_' . $entity_type]['auto_attach'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Automatic field replacement'),
      '#options' => $options,
      '#default_value' => !empty($default['auto_attach']) ? $default['auto_attach'] : array(),
      '#description' => t('Automatically replace the selected field(s) when creating a new bundle.'),
    );

    $form['settings']['title_' . $entity_type]['hide_label'] = _title_hide_label_widget($default, $info['label']);
  }

  // Add a submit button
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Submit handler for title_admin_settings_form().
 */
function title_admin_settings_form_submit($form, &$form_state) {
  $config = config('title.settings');
  foreach(entity_get_info() as $entity_type => $info) {
    $config->set('title_' . $entity_type, $form_state['values']['title_' . $entity_type]);
    $config->set('title_' . $entity_type['auto_attach'], $form_state['values']['title_' . $entity_type]['auto_attach']);
  }
  $config->save();
  backdrop_set_message(t('The configuration options have been saved.'));
}
