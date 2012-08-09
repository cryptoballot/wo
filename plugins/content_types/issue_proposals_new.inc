<?php
$plugin = array(
  'title'            => t('Issue - Proposals - New'),
  'description'      => t('Displays small sized issue block'),
  'single'           => TRUE,
  'category'         => t('Wikiocracy'),
  'required context' => new ctools_context_required(t('Issue'), 'node'),
  'render callback'  => 'wo_issue_proposals_new_render',
);

function wo_issue_proposals_new_render($subtype, $conf, $panel_args, &$context){
  $node = $context->data;
  
  $block = new stdClass();
  $block->title = '';
  $block->content = '<div class="wo-issue-proposals-new wo-issue-proposals wo-proposals-list">';

  $query = new EntityFieldQuery();
  $result = $query->entityCondition('entity_type', 'node')
    ->propertyCondition('type', 'wo_proposal')
    ->fieldCondition('field_wo_proposal_issue', 'target_id', $node->nid)
    ->propertyOrderBy('created', 'DESC')
    ->execute();

  foreach ($result['node'] as $item) {
    $proposal = node_load($item->nid);
    $block->content .= '<div class="proposal">';
    $block->content .= '<h3>' . $proposal->title . '</h3>';
    $block->content .= '<div class="body">' . $proposal->field_wo_proposal_body['und'][0]['safe_value'] . '</div>';
    $block->content .= '<div class="learnmore">' . l('Learn More', 'node/' . $proposal->nid) . '</div>';
    $block->content .= '</div>';
  }

  $block->content .= '</div>';
  return $block;
}
