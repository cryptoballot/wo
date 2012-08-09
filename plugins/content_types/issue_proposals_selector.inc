<?php
$plugin = array(
  'title'            => t('Issue - Proposals - View Selector'),
  'description'      => t('Displays links to top / new / hot'),
  'single'           => TRUE,
  'category'         => t('Wikiocracy'),
  'required context' => new ctools_context_required(t('Issue'), 'node'),
  'render callback'  => 'wo_issue_proposals_selector_render',
);

function wo_issue_proposals_selector_render($subtype, $conf, $panel_args, &$context){
  $node = $context->data;
  
  $block = new stdClass();
  $block->title = '';
  $block->content = '<div class="wo-issue-proposals-selector">';

  $items = array(
    l('top', 'node/' . $node->nid, array('query' => array('view' => 'top'))),
    l('new', 'node/' . $node->nid, array('query' => array('view' => 'new'))),
    l('hot', 'node/' . $node->nid, array('query' => array('view' => 'hot'))),
  );

  $block->content .= theme('item_list', array('items' => $items, 'attributes' => array('class' => 'inline')));

  $block->content .= '</div>';
  return $block;
}
