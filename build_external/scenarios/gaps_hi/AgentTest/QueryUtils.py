"""
## Netdata web api query options tests
# chart name
# before
# after
# group
# points
# context
# chart_label_key
# dimensions
# gtime
# format
# options
# filename

extern int web_client_api_request_v1_charts(RRDHOST *host, struct web_client *w, char *url);
extern int web_client_api_request_v1_archivedcharts(RRDHOST *host, struct web_client *w, char *url);
extern int web_client_api_request_v1_chart(RRDHOST *host, struct web_client *w, char *url);
extern int web_client_api_request_v1_data(RRDHOST *host, struct web_client *w, char *url);
extern int web_client_api_request_single_chart(RRDHOST *host, struct web_client *w, char *url, void callback(RRDSET *st, BUFFER *buf));

extern int web_client_api_request_v1_registry(RRDHOST *host, struct web_client *w, char *url);
extern int web_client_api_request_v1_info(RRDHOST *host, struct web_client *w, char *url);
extern int web_client_api_request_v1(RRDHOST *host, struct web_client *w, char *url);
extern int web_client_api_request_v1_info_fill_buffer(RRDHOST *host, BUFFER *wb);
"""
import time

def prepare_get_data_options():
    """ API Query - c-code int web_client_api_request_v1_data(RRDHOST *host, struct web_client *w, char *url); 
    Query parameters,
    Relative: after=-600&before=0
    Absolute: after=<unix timestamp>&before=<unix timestamp>
    group: <=0 - NotValid and handled by the agent, =1 mean no grouping, >1 grouping values/resampling/downsampling
    points: specific number of requested points
    options: options=unaligned|allow_past|absolute
    """
    pass

def exercise_after_before(max_update_every=5, absolute=False):
    if ((max_update_every) or (max_update_every <= 0) or (absolute)) is None:
        print(f"Please provide valid arguments to the exercise_after_before() . Not max_update_every={max_update_every} OR absolute={absolute}")
    # test no samples
    # test 1 sample
    # test 2 samples
    # test first sample in 1 sample
    # test first sample in 2 samples
    # before == after == 0
    # before < after 
    # before > after
    # before == after
    if absolute:
        before = int(time.time())
    else:
        before = 0
    test_cases = dict() 
    for ue in range(0, max_update_every + 1):
        after = before - ue
        url_params = {"after":after, "before":before}
        test_cases[f"ue_{ue}"] = url_params
        # print(url_params)
    return test_cases

def per_sample_after_before(chart_info, numofsamples = 1, absolute=False):
    if ((chart_info) or (numofsamples < 0) or (absolute)) is None:
        print(f"Please provide valid arguments to the exercise_after_before() . Not chart_info={str(chart_info)} OR #samples={numofsamples} OR absolute={absolute}")
    update_every = chart_info["update_every"]
    first_entry = chart_info["first_entry"]
    last_entry = chart_info["last_entry"]
    duration = chart_info["duration"]
    if absolute:
        before = last_entry
        after = first_entry
    else:
        before = 0
        after = -duration
    if numofsamples > duration:
        print("Warning: Requested number of samples is bigger than the chart duration. numofsamples = duration!")
        numofsamples = duration
    if (numofsamples*update_every) <= duration: 
        before = after + (numofsamples*update_every)
    url_params = {"after": after, "before": before}
    return url_params

def for_sample_after_before(chart_info, numofsamples = 1, absolute=False):
    if ((chart_info) or (numofsamples < 0) or (absolute)) is None:
        print(f"Please provide valid arguments to the exercise_after_before() . Not chart_info={str(chart_info)} OR #samples={numofsamples} OR absolute={absolute}")
    update_every = chart_info["update_every"]
    first_entry = chart_info["first_entry"]
    last_entry = chart_info["last_entry"]
    duration = chart_info["duration"]
    if absolute:
        before = last_entry
        after = first_entry
    else:
        before = 0
        after = -duration
    if numofsamples > duration:
        print("Warning: Requested number of samples is bigger than the chart duration. numofsamples = duration!")
        numofsamples = duration
    test_cases = dict() 
    url_params = {"after": after, "before": before}
    test_cases[f"ue_s0"] = url_params
    for i in range(1, (numofsamples + 1)):
        before = after + (i*update_every)
        url_params = {"after": after, "before": before}
        test_cases[f"ue_s{i}"] = url_params
    return test_cases

# tcs = exercise_after_before(1, False)
# tcs_abs = exercise_after_before(1, True)
# print(tcs)
# print(tcs_abs)