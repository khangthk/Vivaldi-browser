// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import './strings.m.js';

import type {ClickInfo} from 'chrome://resources/js/browser_command.mojom-webui.js';
import {Command} from 'chrome://resources/js/browser_command.mojom-webui.js';
import {BrowserCommandProxy} from 'chrome://resources/js/browser_command/browser_command_proxy.js';
import {EventTracker} from 'chrome://resources/js/event_tracker.js';
import {isChromeOS} from 'chrome://resources/js/platform.js';
import {CrLitElement} from 'chrome://resources/lit/v3_0/lit.rollup.js';

import {getCss} from './whats_new_app.css.js';
import {getHtml} from './whats_new_app.html.js';
import {WhatsNewProxyImpl} from './whats_new_proxy.js';

interface CommandData {
  commandId: number;
  clickInfo: ClickInfo;
}

// TODO (https://www.crbug.com/1219381): Add some additional parameters so
// that we can filter the messages a bit better.
interface BrowserCommandMessageData {
  data: CommandData;
}

export class WhatsNewAppElement extends CrLitElement {
  static get is() {
    return 'whats-new-app';
  }

  static override get styles() {
    return getCss();
  }

  override render() {
    return getHtml.bind(this)();
  }

  static override get properties() {
    return {
      url_: {type: String},
    };
  }

  protected url_: string = '';

  private isAutoOpen_: boolean = false;
  private eventTracker_: EventTracker = new EventTracker();

  constructor() {
    super();

    const queryParams = new URLSearchParams(window.location.search);
    this.isAutoOpen_ = queryParams.has('auto');

    // There are no subpages in What's New. Also remove the query param here
    // since its value is recorded.
    window.history.replaceState(undefined /* stateObject */, '', '/');
  }

  override connectedCallback() {
    super.connectedCallback();

    WhatsNewProxyImpl.getInstance().initialize().then(
        url => this.handleUrlResult_(url));
  }

  override disconnectedCallback() {
    super.disconnectedCallback();
    this.eventTracker_.removeAll();
  }

  /**
   * Handles the URL result of sending the initialize WebUI message.
   * @param url The What's New URL to use in the iframe.
   */
  private handleUrlResult_(url: string|null) {
    if (!url) {
      // This occurs in the special case of tests where we don't want to load
      // remote content.
      return;
    }

    const latest = this.isAutoOpen_ && !isChromeOS ? 'true' : 'false';
    url += url.includes('?') ? '&' : '?';
    this.url_ = url.concat(`latest=${latest}`);

    this.eventTracker_.add(
        window, 'message',
        (event: Event) => this.handleMessage_(event as MessageEvent));
  }

  private handleMessage_(event: MessageEvent) {
    if (!this.url_) {
      return;
    }

    const {data, origin} = event;
    const iframeUrl = new URL(this.url_);
    if (!data || origin !== iframeUrl.origin) {
      return;
    }

    const commandData = (data as BrowserCommandMessageData).data;
    if (!commandData) {
      return;
    }

    const commandId = Object.values(Command).includes(commandData.commandId) ?
        commandData.commandId :
        Command.kUnknownCommand;

    const handler = BrowserCommandProxy.getInstance().handler;
    handler.canExecuteCommand(commandId).then(({canExecute}) => {
      if (canExecute) {
        handler.executeCommand(commandId, commandData.clickInfo);
      } else {
        console.warn('Received invalid command: ' + commandId);
      }
    });
  }
}
customElements.define(WhatsNewAppElement.is, WhatsNewAppElement);
