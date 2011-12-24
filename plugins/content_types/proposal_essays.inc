<?php
$plugin = array(
  'title'            => t('Proposal Essays'),
  'description'      => t('Displays proposal essay teasers and links'),
  'single'           => TRUE,
  'category'         => t('Wikiocracy'),
  'required context' => new ctools_context_required(t('Node'), 'node'),
  'render callback'  => 'wo_proposal_essays_render',
);

function wo_proposal_essays_render($subtype, $conf, $panel_args, &$context){
  $block = new stdClass();
  $block->title = '';
  $block->content = '<div class="wo-proposal-essays">';
  
  $proposal = $context->data;
  
  $block->content .= '<div class="wo-proposal-essay-for">';
  if ($proposal->field_wo_proposal_essay_for) {
    $essay_for = node_load($proposal->field_wo_proposal_essay_for['und'][0]['nid']);
    $block->content .= $essay_for->body['und'][0]['safe_summary'];
  }
  else {
    $block->content .= 'The essay speaking <strong>for</strong> this proposal has not yet been created. '.l('Create It','node/add/wo-essay/for/'.$proposal->nid);
  }
  $block->content .= '</div>';

  $block->content .= '<div class="wo-proposal-essay-against">';  
  if ($proposal->field_wo_proposal_essay_agt) {
    $essay_against = node_load($proposal->field_wo_proposal_essay_agt['und'][0]['nid']);
    $block->content .= $essay_against->body['und'][0]['safe_summary'];
  }
  else {
    $block->content .= 'The essay speaking <strong>against</strong> this proposal has not yet been created. '.l('Create It','node/add/wo-essay/for/'.$proposal->nid);
  }
  $block->content .= '</div>';
  
  $block->content .= '</div>';
  return $block;
}
