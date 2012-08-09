<?php
$plugin = array(
  'title'            => t('Issue - Preview'),
  'description'      => t('Displays small sized issue block'),
  'single'           => TRUE,
  'category'         => t('Wikiocracy'),
  'required context' => new ctools_context_required(t('Issue'), 'node'),
  'render callback'  => 'wo_issue_preview_render',
);

function wo_issue_preview_render($subtype, $conf, $panel_args, &$context){
  global $user;

  $node = $context->data;

  // Get the number of votes
  $query = new EntityFieldQuery();
  $num_votes = $query->entityCondition('entity_type', 'node')
    ->propertyCondition('type', 'wo_vote')
    ->fieldCondition('field_wo_vote_issue', 'target_id', $node->nid)
    ->count()
    ->execute();

  // Get the users current vote
  $query = new EntityFieldQuery();
  $result = $query->entityCondition('entity_type', 'node')
    ->propertyCondition('type', 'wo_vote')
    ->propertyCondition('uid', $user->uid)
    ->fieldCondition('field_wo_vote_issue', 'target_id', $node->nid)
    ->execute();

  if (isset($result['node'])) {
    $vote_nid = array_pop($result['node'])->nid;
  }
  else {
    $vote_nid = FALSE;
  }

  // Mark transitivity
  $trans = (bool) $node->field_wo_issue_transitive['und'][0]['value'];

  // Mark consensus in percentage
  $consensus = round(100 * $node->field_wo_issue_consensus['und'][0]['value']);
  
  $block = new stdClass();
  $block->title = '';
  $block->content = '<div class="wo-issue-preview">';


  $block->content .= '<div class="top-bar">';

  // Issue Title
  $block->content .= '<h2>' . $node->title . '</h2>';

  $block->content .= '<div class="action-group">';

  // Vote Count
  $block->content .= '<div class="votes">';
  $block->content .= '<span class="label">Votes:</span><span class="data"> ' . $num_votes . '</span>';
  $block->content .= '</div>';

  // Consensus Indicator
  $block->content .= '<div class="consensus ' . ($trans ? 'transitive' : 'not-transitive') . '">';
  $block->content .= '<span class="label">Consensus:</span><span class="data"> ' . $consensus . '%</span>';
  $block->content .= '</div>';

  // Vote Button
  $block->content .= '<div class="vote-button">';
  if ($vote_nid) {
    $block->content .= l('Modify Vote','node/' . $vote_nid . '/edit');
  }
  else {
    $block->content .= l('Vote','node/add/wo-vote/' . $node->nid);
  }
  $block->content .= '</div>';

  // New Proposal Button
  $block->content .= '<div class="new-proposal-button">';
  $block->content .= l('New Proposal','node/add/wo-proposal/' . $node->nid);
  $block->content .= '</div>';

  $block->content .= '</div>'; // End action-group
  $block->content .= '</div>'; // End top bar


  $block->content .= '<div class="description">';
  $block->content .= $node->field_wo_issue_desc['und'][0]['safe_value'];
  $block->content .= '</div>'; // End description

  $block->content .= '</div>';
  return $block;
}
