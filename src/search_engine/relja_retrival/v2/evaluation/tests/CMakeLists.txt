add_executable( compute_eval_oxford compute_eval_oxford.cpp )
target_link_libraries( compute_eval_oxford evaluator_v2 )

add_executable( compute_oxGQ compute_oxGQ.cpp )
target_link_libraries( compute_oxGQ
    feat_standard
    hamming
    hamming_embedder
    tfidf_v2
    ${fastann_LIBRARIES} )

add_executable( compute_Pitts_query_feats compute_Pitts_query_feats.cpp )
target_link_libraries( compute_Pitts_query_feats
    feat_standard
    hamming
    hamming_embedder
    tfidf_v2
    ${fastann_LIBRARIES} )

add_executable( compute_SF_query_feats compute_SF_query_feats.cpp )
target_link_libraries( compute_SF_query_feats
    feat_standard
    hamming
    hamming_embedder
    tfidf_v2
    ${fastann_LIBRARIES} )
