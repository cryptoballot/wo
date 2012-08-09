<?php
/**
 * Batch operations
 * 
 * These functions are run occationally (on cron) to update various metrices such 
 * as consensus ratings, vote totals etc.
 */

function wo_batch_update_issue($issue) {
  $results = wo_calculate_vote($issue->nid);
  $issue->field_wo_issue_proposals['und'] = array();
  foreach ($results['list'] as $proposal_nid) {
    $issue->field_wo_issue_proposals['und'][]['target_id'] = $proposal_nid;
  }
  $issue->field_wo_issue_consensus['und'][0]['value'] = $results['consensus'];

  // Assign transitivity
  if ($results['winner'] === FALSE) {
    $issue->field_wo_issue_transitive['und'][0]['value'] = 0;
  }
  else {
    $issue->field_wo_issue_transitive['und'][0]['value'] = 1;
  }

  node_save($issue);
}