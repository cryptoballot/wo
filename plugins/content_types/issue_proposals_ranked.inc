<?php
$plugin = array(
  'title'            => t('Issue - Proposals - Ranked'),
  'description'      => t('Displays small sized issue block'),
  'single'           => TRUE,
  'category'         => t('Wikiocracy'),
  'required context' => new ctools_context_required(t('Issue'), 'node'),
  'render callback'  => 'wo_issue_proposals_ranked_render',
);

function wo_issue_proposals_ranked_render($subtype, $conf, $panel_args, &$context){
  $node = $context->data;
  
  $block = new stdClass();
  $block->title = '';
  $block->content = '<div class="wo-issue-proposals-ranked wo-issue-proposals wo-proposals-list">';

  foreach ($node->field_wo_issue_proposals['und'] as $item) {
    $proposal = node_load($item['target_id']);
    $block->content .= '<div class="proposal">';
    $block->content .= '<h3>' . l($proposal->title, 'node/' . $proposal->nid) . '</h3>';
    $block->content .= '<div class="body">' . $proposal->field_wo_proposal_body['und'][0]['safe_value'] . '</div>';
    $block->content .= '<div class="learnmore">' . l('Learn More', 'node/' . $proposal->nid) . '</div>';
    $block->content .= '</div>';
  }

  $block->content .= '</div>';
  return $block;
}
