import EmberRouter from '@ember/routing/router';
import config from 'active-directory-dashboard/config/environment';

export default class Router extends EmberRouter {
  location = config.locationType;
  rootURL = config.rootURL;
}

Router.map(function () {
  this.route('home');
  this.route('users');
  this.route('file');
  this.route('computers');
  this.route('ou');
  this.route('group');
  this.route('deleted-objects');
});
