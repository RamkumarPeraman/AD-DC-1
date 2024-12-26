import Controller from '@ember/controller';
import { action } from '@ember/object';
import { tracked } from '@glimmer/tracking';

export default class OUController extends Controller {
  @tracked ous = [];
  @tracked selectedOU = null;

  @action
  showOUDetails(ou) {
    this.selectedOU = ou;
  }

  @action
  closePopup() {
    this.selectedOU = null;
  }
}