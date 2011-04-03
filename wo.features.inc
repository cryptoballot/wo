<?php
/**
 * @file
 * wo.features.inc
 */

/**
 * Implementation of hook_node_info().
 */
function wo_node_info() {
  $items = array(
    'wo_essay' => array(
      'name' => t('Essay'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
    'wo_issue' => array(
      'name' => t('Issue'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
    'wo_proposal' => array(
      'name' => t('Proposal'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
    'wo_vote' => array(
      'name' => t('Vote'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  return $items;
}
