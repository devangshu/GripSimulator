/* GRIP SIMULATOR */
// web client

var main, ws, ui;

ui = {
    colors: {},
    update_virtual_pin: (vpin_num, vpin_val) => {
        ui.log(`updating ui for vp${vpin_num}: ${vpin_val}`);
        // TODO: actually update ui here
    },
    init: (resolve) => {
        ui.log('initializing');
        if (resolve) resolve();
    },
    log: util.logger('ui'),
    err: util.logger('ui', true)
};

ws = {
    id: 0,
    socket: null,
    events: [],
    url: (location.protocol === 'https:' ? 'wss://' : 'ws://') + document.domain +
        (document.domain == 'localhost' ? ':8080' : ((location.protocol === 'https:' ? ':443' : ':80') + '/socket')),
    bind_events: _ => {
        ws.log("binding events");
        for (var v = 0; v < config.virtualpin_count; v++) {
            ws.bind(`${config.virtualpin_topic}_${v}`, ws.generate_virtualpin_handler(v));
        }
    },
    generate_virtualpin_handler: vp_num => {
        return (vp_value) => {
            main.receive_virtual_pin_update(vp_num, parseInt(vp_value), vp_value);
        };
    },
    encode_msg: (e, d) => {
        return JSON.stringify({
            event: e,
            data: d
        });
    },
    decode_msg: (m) => {
        try {
            m = JSON.parse(m);
        } catch (e) {
            ws.err('invalid json ', e);
            m = null;
        }
        return m;
    },
    bind: (event, handler) => {
        ws.events[event] = handler;
    },
    connect: resolve => {
        ws.log('connecting');
        var socket = new WebSocket(ws.url);
        socket.addEventListener('open', _ => {
            ws.log(`socket connected to ${ws.url}`);
            if (resolve) resolve();
        });
        socket.addEventListener('error', e => {
            ws.err('socket error', e.data);
        });
        socket.addEventListener('message', e => {
            var d = ws.decode_msg(e.data);
            if (d != null) {
                ws.log('socket received:', d.event, d.data);
                if (ws.events.hasOwnProperty(d.event) && ws.events[d.event])
                    ws.events[d.event](d.data);
            } else {
                ws.err('socket received:', 'invalid message', e.data);
            }
        });
        socket.addEventListener('close', e => {
            ws.log('socket disconnected');
            ws.api.disconnect();
            // alert('disconnected from server');
        });
        window.addEventListener('beforeunload', e => {
            // socket.close(1001);
        });
        ws.socket = socket;
        ws.bind_events();
    },
    send: (event, data) => {
        ws.log('socket sending:', event, data);
        ws.socket.send(ws.encode_msg(event, data));
    },
    api: {
        cookie_login_flag: false,
        cookie_login: _ => {
            var hash_cookie = util.cookie('password');
            if (hash_cookie != null && (`${hash_cookie}`).trim() != "") {
                ws.api.cookie_login_flag = true;
                hash_cookie = (`${hash_cookie}`).trim();
                ws.send('auth', {
                    password: hash_cookie
                });
            }
        },
        login: password => {
            util.sha256(`${password}`, hash => {
                ws.send('auth', {
                    password: `${hash}`
                });
                util.cookie('password', `${hash}`, '__indefinite__');
            });
        },
        logout: _ => {
            util.delete_cookie('password');
            window.location.reload();
        },
        disconnect: _ => {
            alert("disconnected");
            window.location.reload();
        }
    },
    log: util.logger('ws'),
    err: util.logger('ws', true)
};

main = {
    test: _ => {
        // TODO: add any necessary web client tests here
        // main.log('testing', '123');
    },
    virtual_pins: [], // local pin values
    receive_virtual_pin_update: (vp_num, vp_val, vp_val_buffer = "") => {
        if (vp_num < 0 || vp_num >= config.virtualpin_count) {
            main.err(`invalid virtual pin update on vp?: invalid pin number ${vp_num}`);
            return;
        }
        if (vp_num == NaN) {
            main.err(`invalid virtual pin update on vp${vp_num}: unable to parse value ${vp_val_buffer}`);
            return;
        }
        // check if update is new
        if (vp_val != main.virtual_pins[vp_num]) {
            main.log(`virtual pin update on vp${vp_num}: ${vp_val}`);
            // update local value
            main.virtual_pins[vp_num] = vp_val;
            // update ui elements
            ui.update_virtual_pin(vp_num, vp_val);
        }
    },
    main: _ => {
        util.delay(_ => {
            main.init(_ => {
                util.delay(main.test, 500);
            });
        }, 100);
    },
    init: resolve => {
        console.clear();
        main.log('loading');
        for (var v = 0; v < config.virtualpin_count; v++)
            main.virtual_pins[v] = config.virtualpin_value_init;
        util.delay(_ => {
            main.log('socket connecting');
            ws.connect(_ => {
                util.delay(_ => {
                    ui.init(_ => {
                        main.log('ready');
                        // util.delay(ws.api.cookie_login, 200);
                        if (resolve) resolve();
                    });
                }, 200);
            });
        }, 100);
    },
    log: util.logger('main'),
    err: util.logger('main', true)
};

$(document).ready(main.main);