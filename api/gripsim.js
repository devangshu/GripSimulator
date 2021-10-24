/* GRIP SIMULATOR
 * API Backend
 * EE445L McDermott
 */


/* IMPORTS */
const fs = require("fs");
const path = require("path");
const http = require("http");
const mqtt = require("mqtt");
const websocket = require("ws");
const express = require("express");
const rn = require('random-number');
const readline = require("readline");


/* ENVIRONMENT */
global.args = process.argv.slice(2);
global.env = global.args[0] == "prod" ? "prod" : "dev";
global.config = JSON.parse(fs.readFileSync('./config.json', { encoding: 'utf8', flag: 'r' }));
global.http_port = global.env == "dev" ? 8000 : global.config.http_port;
global.ws_port = global.env == "dev" ? 8080 : global.config.ws_port;

/* MODULES */
var utils, db, web, ws, mq, cli, main;

// utils
utils = {
    delay: (callback, timeout) => {
        setTimeout(_ => {
            process.nextTick(callback);
        }, timeout);
    },
    input: readline.createInterface({
        input: process.stdin,
        output: process.stdout
    }),
    rand_id: (length = 10) => {
        var key = "";
        var chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        for (var i = 0; i < length; i++)
            key += chars[rn({
                min: 0,
                max: chars.length - 1,
                integer: true
            })];
        return key;
    },
    lpad: (s, width, char) => {
        return s.length >= width
            ? s
            : (new Array(width).join(char) + s).slice(-width);
    }, // https://stackoverflow.com/questions/10841773/javascript-format-number-to-day-with-always-3-digits
    capitalize: word => {
        return word.charAt(0).toUpperCase() + word.slice(1);
    },
    rand_int: (low, high) => {
        // inclusive
        return (Math.floor(Math.random() * (high - low + 1)) + low);
    },
    logger: (module, err = false) => {
        return (...args) => {
            args = Array.prototype.slice.call(args);
            args.unshift(`[${module}]`);
            target = err ? console.error : console.log;
            target.apply(null, args);
        }
    },
    is_alphanum_str: "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890",
    is_alphanum: (str) => {
        for (var i = 0; i < str.length; i++) {
            if (!utils.is_alphanum_str.includes(str[i]))
                return false;
        }
        return true;
    }
};

// db
db = {
    api: {
        device_exists: (device_id) => {
            return (db.local.device.hasOwnProperty(device_id) && db.local.device[device_id]);
        },
        set_device_ws_client_id: (device_id, ws_client_id) => {
            db.local.device[device_id].esp_ws_client_id = ws_client_id;
        },
        get_device_ws_client_id: (device_id) => {
            return db.local.device[device_id].esp_ws_client_id;
        },
        get_virtualpin_topic: (vp_num) => {
            return db.local.virtualpin[`vp${vp_num}`].topic;
        },
        get_virtualpin_value: (vp_num) => {
            return db.local.virtualpin[`vp${vp_num}`].value;
        },
        set_virtualpin_value: (vp_num, value) => {
            db.local.virtualpin[`vp${vp_num}`].value = value;
        },
        virtualpin_count: _ => {
            return Object.keys(db.local.virtualpin).length;
        },
    },
    local: {},
    init: (resolve) => {
        db.log("initializing");
        db.local = {
            device: {
                deviceA: {
                    esp_client_type: global.config.defaults.esp_client_type,
                    esp_ws_client_id: null,
                    esp_mqtt_client_id: `${global.config.defaults.esp_mqtt_client_id}_deviceA`
                },
                deviceB: {
                    esp_client_type: global.config.defaults.esp_client_type,
                    esp_ws_client_id: null,
                    esp_mqtt_client_id: `${global.config.defaults.esp_mqtt_client_id}_deviceB`
                }
            },
            virtualpin: {
                vp0: {
                    topic: global.config.defaults.virtualpin_topics[0],
                    value: 0
                },
                vp1: {
                    topic: global.config.defaults.virtualpin_topics[1],
                    value: 0
                },
                vp2: {
                    topic: global.config.defaults.virtualpin_topics[2],
                    value: 0
                },
                vp3: {
                    topic: global.config.defaults.virtualpin_topics[3],
                    value: 0
                },
                vp4: {
                    topic: global.config.defaults.virtualpin_topics[4],
                    value: 0
                },
            }
        };
        if (resolve) resolve();
    },
    log: utils.logger('db'),
    err: utils.logger('db', true),
    exit: resolve => {
        db.log("exit");
        if (resolve) resolve();
    }
};

// web
web = {
    express_api: null,
    http_server: null,
    cors: (req, res, next) => {
        res.header("Access-Control-Allow-Origin", "*");
        res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
        next();
    },
    data_handle: (req, res, dir, file) => {
        var file_path = path.join(__dirname, `${dir}/${file}`);
        if (fs.existsSync(file_path)) {
            res.setHeader('content-type', 'application/octet-stream');
            res.sendFile(file_path);
        } else web.return_error(req, res, 404, `file '${file}' not found`);
    },
    return_error: (req, res, code, msg) => {
        res.status(code);
        res.setHeader('content-type', 'application/json');
        res.send(JSON.stringify({
            status: code,
            success: false,
            message: msg
        }, null, 2));
    },
    init: (resolve) => {
        web.log("initializing");
        web.express_api = express();
        web.express_api.set('view engine', 'ejs');
        web.http_server = http.Server(web.express_api);
        web.express_api.use(express.json());
        web.express_api.use(express.urlencoded({ extended: true }));
        web.express_api.use(web.cors);
        web.express_api.use(express.static("static"));
        web.express_api.get("/", (req, res) => {
            res.render('app');
        });
        web.express_api.listen(global.http_port, _ => {
            web.log("listening on", global.http_port);
            if (resolve) resolve();
        });
    },
    log: utils.logger('web'),
    err: utils.logger('web', true),
    exit: resolve => {
        web.log("exit");
        web.http_server.close(_ => {
            if (resolve) resolve();
        });
    }
};

// ws
ws = {
    socket: null,
    online: false,
    clients: {}, // client sockets
    events: {}, // event handlers
    bind_events: (resolve) => {
        // attach client socket events
        ws.bind('auth', (client, req, db) => {
            // validate password
            if ((`${req.password}`).trim() == global.config.secret) {
                ws.log(`client ${client.id} – authenticated`);
                // set auth in client object
                client.auth = true;
                ws.send_to_client("auth", true, client); // confirm auth with client
            } else ws.send_to_client("auth", false, client);
        }, false);
        ws.bind('identify', (client, req, db) => {
            if ((`${req.password}`).trim() == global.config.secret && db.api.device_exists(req.device_id)) {
                ws.log(`client ${client.id} – identified as device ${req.device_id}`);
                db.api.set_device_ws_client_id(req.device_id, client.id);
                client.auth = true;
                ws.send_to_device_client("identify", true, client);
            } else ws.send_to_device_client("identify", false, client);
        }, false);

        if (resolve) resolve();
    },
    // encode event+data to JSON
    encode_msg: (e, d) => {
        return JSON.stringify({
            event: e,
            data: d
        });
    },
    // decode event+data from JSON
    decode_msg: (m) => {
        try {
            m = JSON.parse(m);
        } catch (e) {
            ws.err("invalid json msg", e);
            m = null;
        }
        return m;
    },
    // send data to specific authenticated non-arduino client
    send_to_client: (event, data, client) => {
        client.socket.send(ws.encode_msg(event, data));
    },
    // send data to specific authenticated non-arduino client
    send_to_device_client: (event, data, client) => {
        client.socket.send(`@${event}-${data}`);
    },
    // send data to all authenticated non-arduino clients
    send_to_clients: (event, data) => {
        for (var c_id in ws.clients) {
            if (
                ws.clients.hasOwnProperty(c_id) &&
                ws.clients[c_id] !== null &&
                ws.clients[c_id].auth
            ) {
                ws.clients[c_id].socket.send(ws.encode_msg(event, data));
            }
        }
    },
    // send data to almost all authenticated non-arduino clients (excluding one)
    send_to_clients_but: (event, data, client) => {
        for (var c_id in ws.clients) {
            if (
                ws.clients.hasOwnProperty(c_id) &&
                c_id != client.id &&
                ws.clients[c_id] !== null &&
                ws.clients[c_id].auth
            ) {
                ws.clients[c_id].socket.send(ws.encode_msg(event, data));
            }
        }
    },
    // bind handler to client event
    bind: (event, handler, auth_req = true) => {
        ws.events[event] = (client, req, db) => {
            if (!auth_req || client.auth)
                handler(client, req, db);
        };
    },
    // initialize & attach events
    initialize: (resolve) => {
        ws.socket = new websocket.Server({ port: global.ws_port });
        // attach server socket events
        ws.socket.on("connection", (client_socket) => {
            // create client object on new connection
            var client = {
                socket: client_socket,
                id: utils.rand_id(),
                auth: false,
                type: "app"
            };
            ws.log(`client ${client.id} – connected`);
            // client socket event handlers
            client.socket.addEventListener("message", (m) => {
                var d = ws.decode_msg(m.data); // parse message
                if (d != null) {
                    // console.log('    ', d.event, d.data);
                    ws.log(`client ${client.id} – message: ${d.event}`, d.data);
                    // handle various events
                    if (ws.events.hasOwnProperty(d.event))
                        ws.events[d.event](client, d.data, db.api);
                    else ws.err("unknown event", d.event);
                } else {
                    ws.err(`client ${client.id} – invalid message: `, m.data);
                }
            });
            client.socket.addEventListener("error", (e) => {
                ws.err("client " + client.id + " – error", e);
            });
            client.socket.addEventListener("close", (c, r) => {
                ws.log(`client ${client.id} – disconnected`);
                delete ws.clients[client.id]; // remove client object on disconnect
            });
            // add client object to client object list
            ws.clients[client.id] = client;
        });
        ws.socket.on("listening", _ => {
            ws.log("listening on", global.ws_port);
            ws.online = true;
            if (resolve) resolve();
        });
        ws.socket.on("error", (e) => {
            ws.log("server error", e);
            ws.online = false;
        });
        ws.socket.on("close", _ => {
            ws.log("server closed");
            ws.online = false;
        });
    },
    init: (resolve) => {
        ws.log("initializing");
        ws.initialize(_ => {
            ws.bind_events(resolve);
        });
    },
    log: utils.logger('ws'),
    err: utils.logger('ws', true),
    exit: resolve => {
        ws.log("exit");
        ws.socket.close();
        if (resolve) resolve();
    }
};

// mqtt
mq = {
    broker: global.config.mqtt_broker_url, // test.mosquitto.org:1883
    publisher: {
        client: null,
        client_options: {
            clientId: global.config.mqtt_publisher_client_id,
        },
        generate_subscribe_callback: topic_name => {
            return (err => {
                if (!err) {
                    mq.log(`publisher subscribed to topic \"${topic_name}\"`);
                } else mq.log(err);
            });
        },
    },
    post: (arg, ch) => {
        mq.log(`publisher posting to topic "${ch}" message: ${arg}`);
        mq.publisher.client.publish(ch, arg);
    },
    events: {}, // event handlers
    bind_events: (resolve) => {
        // attach topic events
        // TODO: bind vp events to update db values
        // TODO: fire relevant websocket handlers as well
        mq.bind(global.config.defaults.virtualpin_topics[0], (topic, message, db) => {
            main.log("update on vp0", message);
        });
        if (resolve) resolve();
    },
    // bind handler to topic event
    bind: (topic, handler) => {
        mq.events[topic] = (message, db) => {
            // check auth here later if necessary
            handler(topic, message.toString(), db);
        };
    },
    init: (resolve) => {
        mq.log("initializing");
        mq.publisher.client = mqtt.connect(`mqtt://${mq.broker}`, mq.publisher.client_options);
        mq.publisher.client.on('connect', _ => {
            mq.log(`publisher connected to broker ${mq.broker}`);
            for (var i = 0; i < db.api.virtualpin_count(); i++) {
                var topic_name = db.api.get_virtualpin_topic(i);
                mq.publisher.client.subscribe(topic_name, mq.publisher.generate_subscribe_callback(topic_name));
            }
            mq.bind_events();
        });
        mq.publisher.client.on('message', (topic, message) => {
            mq.log(`subscriber received message on topic \"${topic.toString()}\":`, message.toString());
            if (mq.events.hasOwnProperty(topic))
                mq.events[topic](message, db.api);
            else mq.err("unknown event", topic);
        });
        if (resolve) resolve();
    },
    log: utils.logger('mqtt'),
    err: utils.logger('mqtt', true),
    exit: resolve => {
        mq.log("exit");
        mq.client.end();
        if (resolve) resolve();
    }
};

// cli
cli = {
    init: resolve => {
        cli.log("initializing");
        utils.input.on('line', (line) => {
            var line_text = '';
            line = line.trim();
            if (line != '') {
                line_text = line;
                line = line.split(' ');
                if (line[0] == "db" || line[0] == "database") {
                    if (line[1] == "save") {
                        // database.save(line[2] && line[2] == "pretty");
                    }
                } else if (line[0] == "test") {
                    cli.log('running tests');
                    main.test();
                } else if (line[0] == "code") {
                    if (line.length > 1 && line[1] != "") {
                        line_text = line_text.substring(4);
                        var ret = eval(line_text);
                        if (ret !== undefined) cli.log(ret);
                    }
                } else if (line[0] == "clear" || line[0] == "c") {
                    console.clear();
                } else if (line[0] == "exit" || line[0] == "quit" || line[0] == "q") {
                    main.exit(_ => {
                        cli.log("bye");
                    }, 0);
                }
            }
        });
        if (resolve) resolve();
    },
    log: utils.logger('cli'),
    err: utils.logger('cli', true),
}

// main
main = {
    test: _ => {
        // TODO: add any necessary server tests here
        // main.log('testing', '123');
    },
    main: _ => {
        process.on('exit', _ => {
            main.log('process:exit');
        });
        process.on('SIGINT', _ => {
            main.log('process:interrupt');
            main.exit();
        });
        process.on('SIGUSR1', _ => {
            main.log('process:restart-1');
            main.exit();
        });
        process.on('SIGUSR2', _ => {
            main.log('process:restart-2');
            main.exit();
        });
        utils.delay(_ => {
            main.init(_ => {
                utils.delay(main.test, 500);
            });
        }, 500);
    },
    init: resolve => {
        main.log("initializing");
        // initialize modules
        mq.init();
        cli.init(_ => {
            db.init(_ => {
                ws.init(_ => {
                    web.init(_ => {
                        main.log("ready");
                        if (resolve) resolve();
                    });
                });
            });
        });
    },
    log: utils.logger('main'),
    err: utils.logger('main', true),
    exit: (resolve, e = 0) => {
        main.log("exit");
        db.exit(_ => {
            mq.exit(_ => {
                ws.exit(_ => {
                    web.exit(_ => {
                        if (resolve) resolve();
                        process.exit(e);
                    });
                });
            });
        });
    }
};


/* MAIN */
console.log("GRIP SIMULATOR");
utils.delay(main.main, 50);